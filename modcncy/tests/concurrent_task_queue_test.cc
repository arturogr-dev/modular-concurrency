// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <modcncy/concurrent_task_queue.h>

#include <chrono>  // NOLINT(build/c++11)
#include <mutex>   // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)
#include <vector>

namespace modcncy {
namespace {

// =============================================================================
TEST(ConcurrentTaskQueueCreationTest, CreateUnsupportedConcurrentQueue) {
  // Setup.
  const auto unsupported_queue_type = static_cast<ConcurrentTaskQueueType>(42);
  auto queue = ConcurrentTaskQueue::Create(unsupported_queue_type);
  // Concurrent queue should not be instantiated.
  EXPECT_EQ(queue, nullptr);
  // Teardown.
  delete queue;
}

class ConcurrentTaskQueueBehaviorTest
    : public testing::TestWithParam<ConcurrentTaskQueueType> {};

INSTANTIATE_TEST_SUITE_P(
    AllConcurrentTaskQueueTypes, ConcurrentTaskQueueBehaviorTest,
    testing::Values(ConcurrentTaskQueueType::kBlockingTaskQueue));

// =============================================================================
TEST_P(ConcurrentTaskQueueBehaviorTest, CreateConcurrentQueue) {
  // Setup.
  auto queue = ConcurrentTaskQueue::Create(/*type=*/GetParam());
  // Concurrent queue should be instantiated successfully.
  EXPECT_NE(queue, nullptr);
  // Teardown.
  delete queue;
}

// =============================================================================
TEST_P(ConcurrentTaskQueueBehaviorTest, ConcurrentTaskQueueExecution) {
  // Setup.
  const int num_threads = std::thread::hardware_concurrency();
  auto queue = ConcurrentTaskQueue::Create(/*type=*/GetParam());
  EXPECT_NE(queue, nullptr);
  std::mutex mutex;
  int counter = 0;  // Guarded by `mutex`.

  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i] {
      // All threads submit a task to increase the counter by 1.
      queue->Push([&] {
        std::unique_lock<std::mutex> lock(mutex);
        ++counter;
      });
      // Sleep only half of the threads to create a more random scenario.
      if (i % 2 == 0) std::this_thread::sleep_for(std::chrono::seconds(1));
      // Each thread pops a task and executes it.
      std::function<void()> task = queue->Pop();
      task();
      // After a "long" period of time, all tasks should have been executed.
      std::this_thread::sleep_for(std::chrono::seconds(2));
      EXPECT_EQ(queue->Pop(), nullptr);
    });
  }

  // Teardown.
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(counter, num_threads);
  delete queue;
}

}  // namespace
}  // namespace modcncy
