// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <modcncy/barrier.h>

#include <algorithm>
#include <chrono>  // NOLINT(build/c++11)
#include <cstring>
#include <memory>
#include <mutex>  // NOLINT(build/c++11)
#include <random>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

namespace modcncy {
namespace {

// =============================================================================
TEST(BarrierCreationTest, CreateUnsupportedBarrier) {
  // Setup.
  const auto unsupported_barrier_type = static_cast<BarrierType>(42);
  auto barrier = Barrier::Create(unsupported_barrier_type);
  // Barrier should not be instantiated.
  EXPECT_EQ(barrier, nullptr);
  // Teardown.
  delete barrier;
}

class BarrierBehaviorTest : public testing::TestWithParam<BarrierType> {};

INSTANTIATE_TEST_SUITE_P(
    AllBarrierTypes, BarrierBehaviorTest,
    testing::Values(BarrierType::kCentralSenseCounterBarrier,
                    BarrierType::kCentralStepCounterBarrier));

// =============================================================================
TEST_P(BarrierBehaviorTest, CreateBarrier) {
  // Setup.
  auto barrier = Barrier::Create(/*type=*/GetParam());
  // Barrier should be instantiated successfully.
  EXPECT_NE(barrier, nullptr);
  // Teardown.
  delete barrier;
}

// =============================================================================
TEST_P(BarrierBehaviorTest, SimpleReadBeforeWrite) {
  // Setup.
  const int num_threads = std::thread::hardware_concurrency();
  auto barrier = Barrier::Create(/*type=*/GetParam());
  EXPECT_NE(barrier, nullptr);
  std::mutex mutex;
  int counter = 0;  // Guarded by `mutex`.

  // Launch `num_threads - 1` threads to hit the barrier and increase `counter`.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (int i = 0; i < num_threads - 1; ++i) {
    threads.emplace_back([&] {
      barrier->Wait(num_threads);
      std::unique_lock<std::mutex> lock(mutex);
      ++counter;
    });
  }

  // Make sure to test the barrier instead of the lock.
  // Pause the main thread so the rest of the threads have a chance to reach the
  // barrier and check that the counter is still 0 since no other thread should
  // pass the barrier.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  {
    std::unique_lock<std::mutex> lock(mutex);
    EXPECT_EQ(counter, 0);
  }
  // Main thread reaches the barrier, unblocking the rest of the threads.
  {
    barrier->Wait(num_threads);
    std::unique_lock<std::mutex> lock(mutex);
    ++counter;
  }

  // Teardown.
  // Wait for the rest of the threads to complete.
  // All threads should have incremented the counter by now.
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(counter, num_threads);
  delete barrier;
}

// =============================================================================
TEST_P(BarrierBehaviorTest, SimpleReadAfterWrite) {
  // Setup.
  auto barrier = Barrier::Create(/*type=*/GetParam());
  EXPECT_NE(barrier, nullptr);
  int shared_variable = 0;  // Variable to protect.
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  // All reader threads should read the most recent value stored by the writer
  // thread successfully.
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads.emplace_back([&, thread_index] {
      if (thread_index == 0) {
        // Only the first thread will perform a store operation.
        shared_variable = 1;
        // Make sure it synchronizes with all other threads.
        barrier->Wait(num_threads);
      } else {
        // All other threads wait until first thread updates the variable.
        barrier->Wait(num_threads);
        // And then, safely read the updated value.
        EXPECT_EQ(shared_variable, 1);
      }
    });
  }

  // Teardown.
  for (auto& thread : threads) thread.join();
  delete barrier;
}

// =============================================================================
TEST_P(BarrierBehaviorTest, ReadAfterWriteByPartialSums) {
  // Setup.
  auto barrier = Barrier::Create(/*type=*/GetParam());
  EXPECT_NE(barrier, nullptr);
  constexpr uint64_t size = 1000000;
  std::vector<uint64_t> data;
  data.reserve(size);
  for (uint64_t i = 1; i <= size; ++i) data.push_back(i);
  const uint64_t expected_sum = size * (size + 1) / 2;
  uint64_t computed_sum = 0;  // Compute total sum here.

  // The goal is to compute the sum of all numbers from 1 to `size`, where we
  // know that the expected result is given by the formula `(size*(size+1))/2`.
  // Each thread computes a partial sum and only the first thread computes the
  // total sum if and only if all other threads have finished their respective
  // partial sums.
  constexpr int num_threads = 16;
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  std::vector<uint64_t> partial_sums(num_threads, 0);
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads.emplace_back([&, thread_index] {
      // Data is divided among execution threads.
      const uint64_t size_per_thread = size / num_threads;
      const uint64_t begin = thread_index * size_per_thread;
      const uint64_t end = begin + size_per_thread;
      // Each thread computes its designated partial sum.
      for (uint64_t i = begin; i < end; ++i)
        partial_sums[thread_index] += data[i];
      // Wait until all other threads finish their respective partial sums.
      barrier->Wait(num_threads);
      // Only the first thread will compute the total sum.
      // If a thread finished computing its partial sum, return.
      if (thread_index > 0) return;
      // The first thread computes the total sum.
      for (auto partial_sum : partial_sums) computed_sum += partial_sum;
      // First thread should have been able to safely compute the total sum.
      // This is it read correctly all the partial results from the other
      // theads. It verifies that the computed sum is equal to the expected sum.
      EXPECT_EQ(computed_sum, expected_sum);
    });
  }

  // Teardown.
  for (auto& thread : threads) thread.join();
  delete barrier;
}

// =============================================================================
TEST_P(BarrierBehaviorTest, ReusableAndAdaptableBarrierBySortingSegments) {
  // Setup.
  auto barrier = Barrier::Create(/*type=*/GetParam());
  EXPECT_NE(barrier, nullptr);
  constexpr int size = 1000000;
  std::vector<int> data;
  data.reserve(size);
  for (int i = 1; i <= size; ++i) data.push_back(i);
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);

  // Helper to verify if data is sorted.
  auto IsSorted = [](const std::vector<int>& data) {
    for (int i = 1; i < size; ++i)
      if (data[i - 1] > data[i]) return false;
    return true;
  };  // function IsSorted
  EXPECT_FALSE(IsSorted(data));

  // Helper to merge a pair of consecutive segments of data of the same size.
  auto Merge = [](int* segment1, int* segment2, int size) {
    // Merge into helper buffer.
    int i = 0, j = 0, k = 0;
    std::vector<int> buffer(2 * size);
    while (i < size && j < size) {
      if (segment1[i] < segment2[j])
        buffer[k++] = segment1[i++];
      else
        buffer[k++] = segment2[j++];
    }
    while (i < size) buffer[k++] = segment1[i++];
    while (j < size) buffer[k++] = segment2[j++];
    // Scatter back into original segments (that are contiguous).
    const int bytes = 2 * size * sizeof(int);
    std::memcpy(/*dst=*/segment1, /*src=*/buffer.data(), /*cnt=*/bytes);
  };  // function Merge

  // The goal is to sort a list of numbers. Each thread sorts a segment of data
  // which will afterwards will be merged with another sorted segment if and
  // only if both of them are already sorted. In the end, the first thread will
  // perform the last merge of two sorted segments to sort the entire list.
  constexpr int num_threads = 16;
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads.emplace_back([&, thread_index] {
      // Data is divided among execution threads.
      int segment_size = size / num_threads;
      const int begin = thread_index * segment_size;
      const int end = begin + segment_size;
      // Each thread sorts its designated chunk of data.
      std::sort(data.begin() + begin, data.begin() + end);
      // Wait until all other threads finish sorting their respective segments.
      barrier->Wait(num_threads);
      // Merging tree.
      int step = 2;
      while (step <= num_threads) {
        // Only even threads will merge a pair of segments.
        // Therefore, a thread returns if it does not have more work to do.
        if (thread_index % step != 0) return;
        // Merge a pair of segments.
        Merge(/*segment1=*/&data[begin],
              /*segment2=*/&data[begin + segment_size],
              /*size=*/segment_size);
        // Continuining threads must wait for the threads performing merges.
        barrier->Wait(num_threads / step);
        // Prepare for next step.
        step *= 2;
        segment_size *= 2;
      }
      // First thread should have been able to safely perform the last merge of
      // segments and verify that the list of numbers is correctly sorted.
      EXPECT_TRUE(IsSorted(data));
    });
  }

  // Teardown.
  for (auto& thread : threads) thread.join();
  delete barrier;
}

}  // namespace
}  // namespace modcncy
