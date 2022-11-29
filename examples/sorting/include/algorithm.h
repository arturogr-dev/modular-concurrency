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

#include "examples/sorting/include/bitonicsort.h"
#include "examples/sorting/include/gnu_impl.h"
#include "examples/sorting/include/oddevensort.h"

namespace sorting {

// Supported execution policies.
enum class SortType {
  kSequentialStdSort = 0,               // GNU sequential std::sort.
  kSequentialOriginalBitonicsort = 1,   // Sequential original bitonicsort.
  kSequentialSegmentedBitonicsort = 2,  // Sequential segment-based bitonicsort.
  kParallelOmpBasedBitonicsort = 3,     // OpenMP-based segment-bitonicsort.
  kParallelBlockingBitonicsort = 4,     // Barrier-based segment-bitonicsort.
  kParallelLockFreeBitonicsort = 5,     // Lock-free segment-bitonicsort.
  kParallelStealingBitonicsort = 6,     // Stealing-barrier segment-bitonicsort.
  kParallelWaitFreeBitonicsort = 7,     // Wait-free segment-bitonicsort.
  kSequentialOriginalOddEvensort = 8,   // Sequential segment-based oddevensort.
  kSequentialSegmentedOddEvensort = 9,  // Sequential segment-based oddevensort.
  kParallelOmpBasedOddEvensort = 10,    // OpenMP-based segment-oddevensort.
  kParallelBlockingOddEvensort = 11,    // Barrier-based segment-oddevensort.
  kParallelLockFreeOddEvensort = 12,    // Lock-free segment-oddevensort.
  kParallelStealingOddEvensort = 13,    // Stealing-barrier segment-oddevensort.
  kParallelWaitFreeOddEvensort = 14,    // Wait-free segment-oddevensort.
  kParallelGnuMultiwayMergesort = 15,   // GNU multiway-mergesort.
  kParallelGnuQuicksort = 16,           // GNU quicksort.
  kParallelGnuBalancedQuicksort = 17,   // GNU balanced-quicksort.
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
      bitonicsort::ompbased(begin, end, num_threads, segment_size);
      break;
    case SortType::kParallelBlockingBitonicsort:
      bitonicsort::blocking(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelLockFreeBitonicsort:
      bitonicsort::lockfree(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelStealingBitonicsort:
      bitonicsort::stealing(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelWaitFreeBitonicsort:
      bitonicsort::waitfree(begin, end, num_threads, segment_size);
      break;
    case SortType::kSequentialOriginalOddEvensort:
      oddevensort::original(begin, end);
      break;
    case SortType::kSequentialSegmentedOddEvensort:
      oddevensort::segmented(begin, end, segment_size);
      break;
    case SortType::kParallelOmpBasedOddEvensort:
      oddevensort::ompbased(begin, end, num_threads, segment_size);
      break;
    case SortType::kParallelBlockingOddEvensort:
      oddevensort::blocking(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelLockFreeOddEvensort:
      oddevensort::lockfree(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelStealingOddEvensort:
      oddevensort::stealing(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelWaitFreeOddEvensort:
      oddevensort::waitfree(begin, end, num_threads, segment_size);
      break;
    case SortType::kParallelGnuMultiwayMergesort:
      gnu_impl::multiway_mergesort(begin, end, num_threads);
      break;
    case SortType::kParallelGnuQuicksort:
      gnu_impl::quicksort(begin, end, num_threads);
      break;
    case SortType::kParallelGnuBalancedQuicksort:
      gnu_impl::balanced_quicksort(begin, end, num_threads);
      break;
  }
}

}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_
