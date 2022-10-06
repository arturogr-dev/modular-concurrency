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
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_BITONICSORT_INCLUDE_BITONICSORT_H_
#define EXAMPLES_BITONICSORT_INCLUDE_BITONICSORT_H_

#include <omp.h>

#include <algorithm>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/bitonicsort/include/merge.h"

namespace bitonicsort {

// Supported execution policies.
enum class ExecutionPolicy {
  kSequential = 0,  // Sequential behavior, no parallelism.
  kOmpBased = 1,    // Using OpenMP directives and implicit barrier.
};

namespace internal {

static constexpr int kDefaultSegmentSize = 256;  // Elements.

// Functions to implement the proper execution policy.
template <typename Iterator>
void sequential_sort(Iterator begin, Iterator end, int segment_size);
template <typename Iterator>
void parallel_ompbased_sort(Iterator begin, Iterator end, int num_threads,
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
            MergeUp(/*segment1=*/&*(begin + i * segment_size),
                    /*segment2=*/&*(begin + ij * segment_size),
                    /*buffer=*/buffer.data(),
                    /*segment_size=*/segment_size);
          else
            MergeDn(/*segment1=*/&*(begin + i * segment_size),
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
              MergeUp(/*segment1=*/&*(begin + i * segment_size),
                      /*segment2=*/&*(begin + ij * segment_size),
                      /*buffer=*/buffer.data(),
                      /*segment_size=*/segment_size);
            else
              MergeDn(/*segment1=*/&*(begin + i * segment_size),
                      /*segment2=*/&*(begin + ij * segment_size),
                      /*buffer=*/buffer.data(),
                      /*segment_size=*/segment_size);
          }
        }
      }
    }
  }  // pragma omp parallel
}

}  // namespace internal
}  // namespace bitonicsort

#endif  // EXAMPLES_BITONICSORT_INCLUDE_BITONICSORT_H_
