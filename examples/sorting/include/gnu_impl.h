// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// GNU standard library implementations of parallel sorting.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_INCLUDE_GNU_IMPL_H_
#define EXAMPLES_SORTING_INCLUDE_GNU_IMPL_H_

#include <omp.h>

#include <algorithm>
#include <parallel/algorithm>  // NOLINT(build/include_order)

namespace sorting {
namespace gnu_impl {

// =============================================================================
// Parallel multiway mergesort.
template <typename Iterator>
void multiway_mergesort(Iterator begin, Iterator end, size_t num_threads) {
  __gnu_parallel::sort(begin, end,
                       __gnu_parallel::multiway_mergesort_tag(num_threads));
}

// =============================================================================
// Parallel quicksort.
template <typename Iterator>
void quicksort(Iterator begin, Iterator end, size_t num_threads) {
  omp_set_nested(1);
  __gnu_parallel::sort(begin, end, __gnu_parallel::quicksort_tag(num_threads));
}

// =============================================================================
// Parallel balanced quicksort.
template <typename Iterator>
void balanced_quicksort(Iterator begin, Iterator end, size_t num_threads) {
  omp_set_nested(1);
  __gnu_parallel::sort(begin, end,
                       __gnu_parallel::balanced_quicksort_tag(num_threads));
}

}  // namespace gnu_impl
}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_GNU_IMPL_H_
