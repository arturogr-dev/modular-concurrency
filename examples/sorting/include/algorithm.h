// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Sorting implementations.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_
#define EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_

#include <modcncy/wait_policy.h>

#include <algorithm>
#include <functional>
#include <parallel/algorithm>  // NOLINT(build/include_order)

#include "examples/sorting/include/bitonicsort.h"

namespace sorting {

// Supported execution policies.
enum class SortType {
  kSequentialStdSort = 0,               // C++ stdlib sort.
  kSequentialOriginalBitonicsort = 1,   // Original bitonicsort.
  kSequentialSegmentedBitonicsort = 2,  // Segmented bitonicsort.
  kParallelOmpBasedBitonicsort = 3,     // OpenMP-based segmented bitonicsort.
  kParallelPthreadsBitonicsort = 4,     // Barrier-based segmented bitonicsort.
  kParallelNonBlockingBitonicsort = 5,  // Non-blocking segmented bitonicsort.
  kParallelGnuMultiwayMergesort = 6,    // C++ stdlib GNU mergesort.
  kParallelGnuQuicksort = 7,            // C++ stdlib GNU quicksort.
  kParallelGnuBalancedQuicksort = 8,    // C++ stdlib GNU balanced quicksort.
};

// =============================================================================
// Main function to execute the different sorting algorithms.
template <typename Iterator>
void sort(Iterator begin, Iterator end,
          SortType sort_type = SortType::kSequentialStdSort,
          size_t num_threads = std::thread::hardware_concurrency(),
          size_t segment_size = 1 /*number of elements*/,
          std::function<void()> wait_policy = &modcncy::cpu_yield) {
  switch (sort_type) {
    case SortType::kSequentialStdSort:
      std::sort(begin, end);
      break;
    case SortType::kSequentialOriginalBitonicsort:
      bitonicsort::original(begin, end);
      break;
    case SortType::kSequentialSegmentedBitonicsort:
      bitonicsort::segmented(begin, end, segment_size);
      break;
    case SortType::kParallelOmpBasedBitonicsort:
      bitonicsort::parallel_ompbased(begin, end, num_threads, segment_size);
      break;
    case SortType::kParallelPthreadsBitonicsort:
      bitonicsort::parallel_pthreads(begin, end, num_threads, segment_size,
                                     wait_policy);
      break;
    case SortType::kParallelNonBlockingBitonicsort:
      bitonicsort::parallel_nonblocking(begin, end, num_threads, segment_size,
                                        wait_policy);
      break;
    case SortType::kParallelGnuMultiwayMergesort:
      __gnu_parallel::sort(begin, end,
                           __gnu_parallel::multiway_mergesort_tag(num_threads));
      break;
    case SortType::kParallelGnuQuicksort:
      __gnu_parallel::sort(begin, end,
                           __gnu_parallel::quicksort_tag(num_threads));
      break;
    case SortType::kParallelGnuBalancedQuicksort:
      __gnu_parallel::sort(begin, end,
                           __gnu_parallel::balanced_quicksort_tag(num_threads));
      break;
  }
}

}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_
