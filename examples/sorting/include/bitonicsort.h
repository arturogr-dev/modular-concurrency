// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// This is a series of implementations of different versions of the so-called
// bitonicsort algorithm for shared-memory computer architectures.
//
// These implementations are based on `merge` operations on data segments,
// except the original algorithm which is based on `compare-exchange` operations
// on individual data elements.
//
// Initially, for the segmented implementations, all segments are individually
// sorted. After that, each sorted segmented is processed by the bitonic merging
// network. In the end, all the input data is globally sorted.
//
// There are different versions of the algorithm.
//
//   + An implementation of the original bitonicsort algorithm, which is based
//     on `compare-exchange` operations.
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
#include <iterator>
#include <thread>  // NOLINT(build/c++11)
#include <utility>
#include <vector>

#include "examples/sorting/include/merge.h"

namespace sorting {
namespace bitonicsort {

// ===========================================================================
// Original bitonicsort.
template <typename Iterator>
void original(Iterator begin, Iterator end) {
  // Setup.
  const size_t data_size = end - begin;

  // Bitonic sorting network.
  for (size_t k = 2; k <= data_size; k *= 2) {
    for (size_t j = k >> 1; j > 0; j >>= 1) {
      for (size_t i = 0; i < data_size; ++i) {
        const size_t ij = i ^ j;
        if (i < ij) {
          if ((i & k) == 0 && *(begin + i) > *(begin + ij))
            std::swap(*(begin + i), *(begin + ij));
          if ((i & k) != 0 && *(begin + i) < *(begin + ij))
            std::swap(*(begin + i), *(begin + ij));
        }
      }
    }
  }
}

// =============================================================================
// Segmented bitonicsort.
template <typename Iterator>
void segmented(Iterator begin, Iterator end, size_t segment_size) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;
  const size_t buffer_size = 2 * segment_size;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  std::vector<value_type> buffer(buffer_size);

  // Sort each indiviual segment.
  for (size_t i = 0; i < data_size; i += segment_size)
    std::sort(begin + i, begin + i + segment_size);

  // Bitonic merging network.
  for (size_t k = 2; k <= num_segments; k <<= 1) {
    for (size_t j = k >> 1; j > 0; j >>= 1) {
      for (size_t i = 0; i < num_segments; ++i) {
        const size_t ij = i ^ j;
        if (i < ij) {
          if ((i & k) == 0)
            merge::Up(/*segment1=*/&*(begin + i * segment_size),
                      /*segment2=*/&*(begin + ij * segment_size),
                      /*buffer=*/buffer.data(),
                      /*segment_size=*/segment_size);
          else
            merge::Dn(/*segment1=*/&*(begin + i * segment_size),
                      /*segment2=*/&*(begin + ij * segment_size),
                      /*buffer=*/buffer.data(),
                      /*segment_size=*/segment_size);
        }
      }
    }
  }
}

// =============================================================================
// Parallel OpenMP segmented bitonicsort.
template <typename Iterator>
void parallel_ompbased(Iterator begin, Iterator end, size_t num_threads,
                       size_t segment_size) {
  // Setup.
  omp_set_dynamic(0);
  omp_set_num_threads(num_threads);

#pragma omp parallel
  {
    const size_t data_size = end - begin;
    const size_t num_segments = data_size / segment_size;
    const size_t buffer_size = 2 * segment_size;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    std::vector<value_type> buffer(buffer_size);

    // Sort each indiviual segment.
#pragma omp for
    for (size_t i = 0; i < data_size; i += segment_size)
      std::sort(begin + i, begin + i + segment_size);

    // Bitonic merging network.
    for (size_t k = 2; k <= num_segments; k <<= 1) {
      for (size_t j = k >> 1; j > 0; j >>= 1) {
#pragma omp for
        for (size_t i = 0; i < num_segments; ++i) {
          const size_t ij = i ^ j;
          if (i < ij) {
            if ((i & k) == 0)
              merge::Up(/*segment1=*/&*(begin + i * segment_size),
                        /*segment2=*/&*(begin + ij * segment_size),
                        /*buffer=*/buffer.data(),
                        /*segment_size=*/segment_size);
            else
              merge::Dn(/*segment1=*/&*(begin + i * segment_size),
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
// Parallel non-blocking segmented bitonicsort.
template <typename Iterator>
void parallel_nonblocking(Iterator begin, Iterator end, size_t num_threads,
                          size_t segment_size) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](Iterator begin, size_t thread_index, size_t num_threads,
                        size_t num_segments, size_t segment_size,
                        std::vector<std::atomic<size_t>>* segment_stage_count) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;
    const size_t buffer_size = 2 * segment_size;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    std::vector<value_type> buffer(buffer_size);
    size_t my_stage = 0;

    for (size_t i = low_index; i < high_index; i += segment_size) {
      // Sort each indiviual segment.
      std::sort(begin + i, begin + i + segment_size);
      // Mark segment "ready" for next stage.
      const size_t segment_id = i / segment_size;
      (*segment_stage_count)[segment_id].fetch_add(1);
    }

    // Mark this thread "ready" for next stage.
    ++my_stage;

    // Bitonic merging network.
    for (size_t k = 2; k <= num_segments; k <<= 1) {
      for (size_t j = k >> 1; j > 0; j >>= 1) {
        for (size_t i = low_segment; i < high_segment; ++i) {
          const size_t ij = i ^ j;
          if (i < ij) {
            const size_t segment1_index = i * segment_size;
            const size_t segment2_index = ij * segment_size;
            const size_t segment1_id = segment1_index / segment_size;
            const size_t segment2_id = segment2_index / segment_size;

            // Wait until the segments I need are on my same stage.
            while (my_stage != (*segment_stage_count)[segment1_id].load())
              modcncy::cpu_yield();
            while (my_stage != (*segment_stage_count)[segment2_id].load())
              modcncy::cpu_yield();

            if ((i & k) == 0)
              merge::Up(/*segment1=*/&*(begin + segment1_index),
                        /*segment2=*/&*(begin + segment2_index),
                        /*buffer=*/buffer.data(),
                        /*segment_size=*/segment_size);
            else
              merge::Dn(/*segment1=*/&*(begin + segment1_index),
                        /*segment2=*/&*(begin + segment2_index),
                        /*buffer=*/buffer.data(),
                        /*segment_size=*/segment_size);

            // Mark segments "ready" for next stage.
            (*segment_stage_count)[segment1_id].fetch_add(1);
            (*segment_stage_count)[segment2_id].fetch_add(1);
          }
        }

        // Mark this thread "ready" for next stage.
        ++my_stage;
      }
    }
  };  // function thread_work

  std::vector<std::atomic<size_t>> segment_stage_count(num_segments);
  for (size_t i = 0; i < num_segments; ++i) segment_stage_count[i] = 0;

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, begin, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  &segment_stage_count));
  }
  thread_work(begin, /*thread_index=*/0, num_threads, num_segments,
              segment_size, &segment_stage_count);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
}

}  // namespace bitonicsort
}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_BITONICSORT_H_
