// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// This is an implementation of the generalized version of the so-called
// bitonicsort algorithm for shared-memory computer architectures. It is based
// on `merge` operations on data segments, instead of `compare-exchange`
// operations on individual data elements. Initially, all segments are
// individually sorted. After that, each sorted segmented is processed by the
// bitonic-merging network. In the end, all the input data is globally sorted.
//
// There are different versions of the algorithm.
//
//   + A sequential (not multithreaded) implementation, where a single execution
//     thread will perform all the merging stages of the bitonic network.
//
//   + An OpenMP-based implementation. The concurrency model is delegated to the
//     OpenMP runtime and its barrier synchronization primitive.
//
//   + A non-blocking multithreaded implementation. Due to the regular memory
//     access pattern that is exposed by the algorithm, it is possible to bypass
//     the explicit use of a synchronization primitive (a barrier in this case).
//     I call this version of the algorithm: self-synchronizable bitonicsort.
//     By exploiting the memory access pattern, one execution thread does not
//     need to wait for all other execution threads to reach the barrier. The
//     idea is to keep track of which data segment is being worked on by which
//     thread during which stage of the algorithm. Therefore, enabling peer to
//     peer synchronization between pair of threads and lock-free progression
//     guarantees with respect to the rest of the execution threads.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_INCLUDE_BITONICSORT_H_
#define EXAMPLES_SORTING_INCLUDE_BITONICSORT_H_

#include <modcncy/wait_policy.h>
#include <omp.h>

#include <algorithm>
#include <atomic>
#include <parallel/algorithm>  // NOLINT(build/include_order)
#include <thread>              // NOLINT(build/c++11)
#include <vector>

#include "examples/sorting/include/merge.h"

namespace bitonicsort {

// Supported execution policies.
enum class ExecutionPolicy {
  kSequential = 0,    // Sequential behavior, no parallelism.
  kOmpBased = 1,      // Using OpenMP directives and implicit barrier.
  kNonBlocking = 2,   // Exploiting the memory access pattern of the algorithm.
  kGnuMergesort = 3,  // To compare against the std multiway mergesort impl.
};

namespace internal {

static constexpr int kDefaultSegmentSize = 256;  // Elements.

template <typename Iterator>
void sequential_sort(Iterator begin, Iterator end, int segment_size);

template <typename Iterator>
void parallel_ompbased_sort(Iterator begin, Iterator end, int num_threads,
                            int segment_size);

template <typename Iterator>
void parallel_nonblocking_sort(Iterator begin, Iterator end, int num_threads,
                               int segment_size);

}  // namespace internal

// =============================================================================
// Main function to execute the different policies.
template <typename Iterator>
void sort(Iterator begin, Iterator end,
          ExecutionPolicy policy = ExecutionPolicy::kSequential,
          int num_threads = std::thread::hardware_concurrency(),
          int segment_size = internal::kDefaultSegmentSize) {
  switch (policy) {
    case ExecutionPolicy::kSequential:
      internal::sequential_sort(begin, end, segment_size);
      break;
    case ExecutionPolicy::kOmpBased:
      internal::parallel_ompbased_sort(begin, end, num_threads, segment_size);
      break;
    case ExecutionPolicy::kNonBlocking:
      internal::parallel_nonblocking_sort(begin, end, num_threads,
                                          segment_size);
      break;
    case ExecutionPolicy::kGnuMergesort:
      __gnu_parallel::sort(begin, end,
                           __gnu_parallel::multiway_mergesort_tag(num_threads));
      break;
  }
}

}  // namespace bitonicsort

////////////////////////////////////////////////////////////////////////////////
///////////////////////  I M P L E M E N T A T I O N S  ////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace bitonicsort {
namespace internal {

// =============================================================================
template <typename Iterator>
void sequential_sort(Iterator begin, Iterator end, int segment_size) {
  // Setup.
  const int data_size = end - begin;
  const int num_segments = data_size / segment_size;
  std::vector<int> buffer(2 * segment_size);

  // Sort each indiviual segment.
  for (int i = 0; i < data_size; i += segment_size)
    std::sort(begin + i, begin + i + segment_size);

  // Bitonic merging network.
  for (int k = 2; k <= num_segments; k <<= 1) {
    for (int j = k >> 1; j > 0; j >>= 1) {
      for (int i = 0; i < num_segments; ++i) {
        const int ij = i ^ j;
        if (i < ij) {
          if ((i & k) == 0)
            merge::MergeUp(/*segment1=*/&*(begin + i * segment_size),
                           /*segment2=*/&*(begin + ij * segment_size),
                           /*buffer=*/buffer.data(),
                           /*segment_size=*/segment_size);
          else
            merge::MergeDn(/*segment1=*/&*(begin + i * segment_size),
                           /*segment2=*/&*(begin + ij * segment_size),
                           /*buffer=*/buffer.data(),
                           /*segment_size=*/segment_size);
        }
      }
    }
  }
}

// =============================================================================
template <typename Iterator>
void parallel_ompbased_sort(Iterator begin, Iterator end, int num_threads,
                            int segment_size) {
  // Setup.
  omp_set_dynamic(0);
  omp_set_num_threads(num_threads);

#pragma omp parallel
  {
    const int data_size = end - begin;
    const int num_segments = data_size / segment_size;
    std::vector<int> buffer(2 * segment_size);

    // Sort each indiviual segment.
#pragma omp for
    for (int i = 0; i < data_size; i += segment_size)
      std::sort(begin + i, begin + i + segment_size);

    // Bitonic merging network.
    for (int k = 2; k <= num_segments; k <<= 1) {
      for (int j = k >> 1; j > 0; j >>= 1) {
#pragma omp for
        for (int i = 0; i < num_segments; ++i) {
          const int ij = i ^ j;
          if (i < ij) {
            if ((i & k) == 0)
              merge::MergeUp(/*segment1=*/&*(begin + i * segment_size),
                             /*segment2=*/&*(begin + ij * segment_size),
                             /*buffer=*/buffer.data(),
                             /*segment_size=*/segment_size);
            else
              merge::MergeDn(/*segment1=*/&*(begin + i * segment_size),
                             /*segment2=*/&*(begin + ij * segment_size),
                             /*buffer=*/buffer.data(),
                             /*segment_size=*/segment_size);
          }
        }
      }
    }
  }  // pragma omp parallel
}

// =============================================================================
template <typename Iterator>
void parallel_nonblocking_sort(Iterator begin, Iterator end, int num_threads,
                               int segment_size) {
  // Setup.
  const int data_size = end - begin;
  const int num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto work = [](Iterator begin, int thread_index, int num_threads,
                 int num_segments, int segment_size,
                 std::vector<std::atomic<int>>* segment_stage_count) {
    // Setup.
    const int num_segments_per_thread = num_segments / num_threads;
    const int low_segment = thread_index * num_segments_per_thread;
    const int high_segment = low_segment + num_segments_per_thread;
    const int low_index = low_segment * segment_size;
    const int high_index = high_segment * segment_size;
    std::vector<int> buffer(2 * segment_size);
    int my_current_stage = 0;

    for (int i = low_index; i < high_index; i += segment_size) {
      // Sort each indiviual segment.
      std::sort(begin + i, begin + i + segment_size);
      // Mark segment "ready" for next stage.
      const int segment_id = i / segment_size;
      (*segment_stage_count)[segment_id].fetch_add(1);
    }

    // Mark this thread "ready" for next stage.
    ++my_current_stage;

    // Bitonic merging network.
    for (int k = 2; k <= num_segments; k <<= 1) {
      for (int j = k >> 1; j > 0; j >>= 1) {
        for (int i = low_segment; i < high_segment; ++i) {
          const int ij = i ^ j;
          if (i < ij) {
            const int segment1_index = i * segment_size;
            const int segment2_index = ij * segment_size;
            const int segment1_id = segment1_index / segment_size;
            const int segment2_id = segment2_index / segment_size;

            // Wait until the segments I need are on my same stage.
            while (my_current_stage !=
                   (*segment_stage_count)[segment1_id].load())
              modcncy::cpu_yield();
            while (my_current_stage !=
                   (*segment_stage_count)[segment2_id].load())
              modcncy::cpu_yield();

            if ((i & k) == 0)
              merge::MergeUp(/*segment1=*/&*(begin + segment1_index),
                             /*segment2=*/&*(begin + segment2_index),
                             /*buffer=*/buffer.data(),
                             /*segment_size=*/segment_size);
            else
              merge::MergeDn(/*segment1=*/&*(begin + segment1_index),
                             /*segment2=*/&*(begin + segment2_index),
                             /*buffer=*/buffer.data(),
                             /*segment_size=*/segment_size);

            // Mark segments "ready" for next stage.
            (*segment_stage_count)[segment1_id].fetch_add(1);
            (*segment_stage_count)[segment2_id].fetch_add(1);
          }
        }

        ++my_current_stage;
      }
    }
  };  // function work

  std::vector<std::atomic<int>> segment_stage_count(num_segments);
  for (int i = 0; i < num_segments; ++i) segment_stage_count[i] = 0;

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (int i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(work, begin, /*thread_index=*/i, num_threads,
                                  num_segments, segment_size,
                                  &segment_stage_count));
  }
  work(begin, /*thread_index=*/0, num_threads, num_segments, segment_size,
       &segment_stage_count);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
}

}  // namespace internal
}  // namespace bitonicsort

#endif  // EXAMPLES_SORTING_INCLUDE_BITONICSORT_H_
