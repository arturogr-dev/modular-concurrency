// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Sorting implementations.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_
#define EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_

#include <algorithm>
#include <parallel/algorithm>  // NOLINT(build/include_order)

#include "examples/sorting/include/bitonicsort.h"

namespace sorting {

// Supported execution policies.
enum class SortType {
  kStdSort = 0,                 // Sequential C++ standard library sort.
  kOriginalBitonicsort = 1,     // Sequential original bitonicsort.
  kSegmentedBitonicsort = 2,    // Sequential segmented bitonicsort.
  kOmpBasedBitonicsort = 3,     // Parallel OpenMP segmented bitonicsort.
  kNonBlockingBitonicsort = 4,  // Parallel non-blocking segmented bitonicsort.
  kGnuMultiwayMergesort = 5,    // Parallel C++ standard library GNU mergesort.
};

// =============================================================================
// Main function to execute the different sorting algorithms.
template <typename Iterator>
void sort(Iterator begin, Iterator end, SortType sort_type = SortType::kStdSort,
          size_t num_threads = std::thread::hardware_concurrency(),
          size_t segment_size = 1 /* number of elements */) {
  switch (sort_type) {
    case SortType::kStdSort:
      std::sort(begin, end);
      break;
    case SortType::kOriginalBitonicsort:
      bitonicsort::original(begin, end);
      break;
    case SortType::kSegmentedBitonicsort:
      bitonicsort::segmented(begin, end, segment_size);
      break;
    case SortType::kOmpBasedBitonicsort:
      bitonicsort::parallel_ompbased(begin, end, num_threads, segment_size);
      break;
    case SortType::kNonBlockingBitonicsort:
      bitonicsort::parallel_nonblocking(begin, end, num_threads, segment_size);
      break;
    case SortType::kGnuMultiwayMergesort:
      __gnu_parallel::sort(begin, end,
                           __gnu_parallel::multiway_mergesort_tag(num_threads));
      break;
  }
}

}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_ALGORITHM_H_
