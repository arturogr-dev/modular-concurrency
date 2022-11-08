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

namespace sorting {

// Supported execution policies.
enum class SortType {
  kSequentialStdSort = 0,               // C++ stdlib sort.
  kSequentialOriginalBitonicsort = 1,   // Original bitonicsort.
  kSequentialSegmentedBitonicsort = 2,  // Segmented bitonicsort.
  kParallelOmpBasedBitonicsort = 3,     // OpenMP-based segmented bitonicsort.
  kParallelBlockingBitonicsort = 4,     // Barrier-based segmented bitonicsort.
  kParallelLockFreeBitonicsort = 5,     // Lock-free segmented bitonicsort.
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
      bitonicsort::ompbased(begin, end, num_threads, segment_size);
      break;
    case SortType::kParallelBlockingBitonicsort:
      bitonicsort::blocking(begin, end, num_threads, segment_size, wait_policy);
      break;
    case SortType::kParallelLockFreeBitonicsort:
      bitonicsort::lockfree(begin, end, num_threads, segment_size, wait_policy);
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
