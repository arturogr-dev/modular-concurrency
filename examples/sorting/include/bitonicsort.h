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
//   + A barrier-based multithreaded implementation. The concurrency is handled
//     via an explicit barrier synchronization primitive, where all threads wait
//     until all others reach this same point (blocking).
//
//   + A non-blocking multithreaded implementation. Due to the regular memory
//     access pattern that is exposed by the algorithm, it is possible to bypass
//     the explicit use of a synchronization primitive (a barrier in this case).
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

#include <modcncy/barrier.h>
#include <modcncy/concurrent_task_queue.h>
#include <modcncy/wait_policy.h>
#include <omp.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <iterator>
#include <thread>  // NOLINT(build/c++11)
#include <utility>
#include <vector>

#include "examples/sorting/include/merge.h"

namespace sorting {
namespace bitonicsort {

// =============================================================================
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
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  value_type* buffer = new value_type[2 * segment_size];

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
                      /*buffer=*/buffer,
                      /*segment_size=*/segment_size);
          else
            merge::Dn(/*segment1=*/&*(begin + i * segment_size),
                      /*segment2=*/&*(begin + ij * segment_size),
                      /*buffer=*/buffer,
                      /*segment_size=*/segment_size);
        }
      }
    }
  }

  delete[] buffer;
}

// =============================================================================
// Parallel OpenMP segmented bitonicsort.
template <typename Iterator>
void ompbased(Iterator begin, Iterator end, size_t num_threads,
              size_t segment_size) {
  // Setup.
  omp_set_dynamic(0);
  omp_set_num_threads(num_threads);

#pragma omp parallel
  {
    const size_t data_size = end - begin;
    const size_t num_segments = data_size / segment_size;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    value_type* buffer = new value_type[2 * segment_size];

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
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);
            else
              merge::Dn(/*segment1=*/&*(begin + i * segment_size),
                        /*segment2=*/&*(begin + ij * segment_size),
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);
          }
        }
      }
    }

    delete[] buffer;
  }  // pragma omp parallel
}

// =============================================================================
// Parallel pthreads segmented bitonicsort.
template <typename Iterator>
void blocking(Iterator begin, Iterator end, size_t num_threads,
              size_t segment_size,
              std::function<void()> wait_policy = &modcncy::cpu_yield) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](Iterator begin, size_t thread_index, size_t num_threads,
                        size_t num_segments, size_t segment_size,
                        std::function<void()> wait_policy,
                        modcncy::Barrier* barrier) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    value_type* buffer = new value_type[2 * segment_size];

    // Sort each indiviual segment.
    for (size_t i = low_index; i < high_index; i += segment_size)
      std::sort(begin + i, begin + i + segment_size);

    barrier->Wait(num_threads, wait_policy);  // Barrier synchronization.

    // Bitonic merging network.
    for (size_t k = 2; k <= num_segments; k <<= 1) {
      for (size_t j = k >> 1; j > 0; j >>= 1) {
        for (size_t i = low_segment; i < high_segment; ++i) {
          const size_t ij = i ^ j;
          if (i < ij) {
            if ((i & k) == 0)
              merge::Up(/*segment1=*/&*(begin + i * segment_size),
                        /*segment2=*/&*(begin + ij * segment_size),
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);
            else
              merge::Dn(/*segment1=*/&*(begin + i * segment_size),
                        /*segment2=*/&*(begin + ij * segment_size),
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);
          }
        }
        barrier->Wait(num_threads, wait_policy);  // Barrier synchronization.
      }
    }

    delete[] buffer;
  };  // function thread_work

  modcncy::Barrier* barrier = modcncy::Barrier::Create(
      modcncy::BarrierType::kCentralSenseCounterBarrier);

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, begin, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  wait_policy, barrier));
  }
  thread_work(begin, /*thread_index=*/0, num_threads, num_segments,
              segment_size, wait_policy, barrier);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  delete barrier;
}

// =============================================================================
// Parallel non-blocking segmented bitonicsort.
template <typename Iterator>
void lockfree(Iterator begin, Iterator end, size_t num_threads,
              size_t segment_size,
              std::function<void()> wait_policy = &modcncy::cpu_yield) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](Iterator begin, size_t thread_index, size_t num_threads,
                        size_t num_segments, size_t segment_size,
                        std::function<void()> wait_policy,
                        std::atomic<size_t>* segment_stage_count) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    value_type* buffer = new value_type[2 * segment_size];
    size_t my_stage = 0;

    for (size_t i = low_index; i < high_index; i += segment_size) {
      // Sort each indiviual segment.
      std::sort(begin + i, begin + i + segment_size);
      // Mark segment "ready" for next stage.
      const size_t segment_id = i / segment_size;
      segment_stage_count[segment_id].fetch_add(1);
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
            while (my_stage != segment_stage_count[segment1_id].load())
              wait_policy();
            while (my_stage != segment_stage_count[segment2_id].load())
              wait_policy();

            if ((i & k) == 0)
              merge::Up(/*segment1=*/&*(begin + segment1_index),
                        /*segment2=*/&*(begin + segment2_index),
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);
            else
              merge::Dn(/*segment1=*/&*(begin + segment1_index),
                        /*segment2=*/&*(begin + segment2_index),
                        /*buffer=*/buffer,
                        /*segment_size=*/segment_size);

            // Mark segments "ready" for next stage.
            segment_stage_count[segment1_id].fetch_add(1);
            segment_stage_count[segment2_id].fetch_add(1);
          }
        }

        // Mark this thread "ready" for next stage.
        ++my_stage;
      }
    }

    delete[] buffer;
  };  // function thread_work

  std::atomic<size_t>* segment_stage_count =
      new std::atomic<size_t>[num_segments];
  for (size_t i = 0; i < num_segments; ++i) segment_stage_count[i] = 0;

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, begin, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  wait_policy, segment_stage_count));
  }
  thread_work(begin, /*thread_index=*/0, num_threads, num_segments,
              segment_size, wait_policy, segment_stage_count);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  delete[] segment_stage_count;
}

// =============================================================================
// Parallel pthreads segmented bitonicsort plus task stealing.
template <typename Iterator>
void stealing(Iterator begin, Iterator end, size_t num_threads,
              size_t segment_size,
              std::function<void()> wait_policy = &modcncy::cpu_yield) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](Iterator begin, size_t thread_index, size_t num_threads,
                        size_t num_segments, size_t segment_size,
                        std::function<void()> wait_policy,
                        modcncy::Barrier* barrier,
                        modcncy::ConcurrentTaskQueue** queue) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;

    auto execute_tasks = [&](size_t queue_index) {
      for (;;) {
        std::function<void()> task = std::move(queue[queue_index]->Pop());
        if (task == nullptr) break;
        task();
      }
    };  // function execute_tasks

    auto steal_tasks = [&]() {
      for (size_t i = thread_index + 1; i < num_threads + thread_index; ++i)
        execute_tasks(/*queue_index=*/i % num_threads);
      wait_policy();
    };  // function steal_tasks

    // Sort each indiviual segment.
    for (size_t i = low_index; i < high_index; i += segment_size) {
      queue[thread_index]->Push(
          [&, i] { std::sort(begin + i, begin + i + segment_size); });
    }
    execute_tasks(thread_index);

    barrier->Wait(num_threads, steal_tasks);  // Barrier synchronization.

    // Bitonic merging network.
    for (size_t k = 2; k <= num_segments; k <<= 1) {
      for (size_t j = k >> 1; j > 0; j >>= 1) {
        // This barrier is necessary to acquire stealed work from other threads.
        barrier->Wait(num_threads, steal_tasks);

        for (size_t i = low_segment; i < high_segment; ++i) {
          const size_t ij = i ^ j;
          if (i < ij) {
            if ((i & k) == 0)
              queue[thread_index]->Push([begin, i, ij, segment_size] {
                typedef typename std::iterator_traits<Iterator>::value_type val;
                // TODO(arturogr-dev): There is room for optimization here.
                // Avoid multiple allocations by allowing arguments in the queue
                // of tasks and, therefore, reuse the same buffer for all calls.
                std::vector<val> buffer(2 * segment_size);
                merge::Up(/*segment1=*/&*(begin + i * segment_size),
                          /*segment2=*/&*(begin + ij * segment_size),
                          /*buffer=*/buffer.data(),
                          /*segment_size=*/segment_size);
              });
            else
              queue[thread_index]->Push([begin, i, ij, segment_size] {
                typedef typename std::iterator_traits<Iterator>::value_type val;
                // TODO(arturogr-dev): There is room for optimization here.
                // Avoid multiple allocations by allowing arguments in the queue
                // of tasks and, therefore, reuse the same buffer for all calls.
                std::vector<val> buffer(2 * segment_size);
                merge::Dn(/*segment1=*/&*(begin + i * segment_size),
                          /*segment2=*/&*(begin + ij * segment_size),
                          /*buffer=*/buffer.data(),
                          /*segment_size=*/segment_size);
              });
          }
        }
        execute_tasks(thread_index);

        // This barrier is necessary to publish stealed work to other threads.
        barrier->Wait(num_threads, steal_tasks);
      }
    }
  };  // function thread_work

  modcncy::Barrier* barrier = modcncy::Barrier::Create(
      modcncy::BarrierType::kCentralSenseCounterBarrier);

  modcncy::ConcurrentTaskQueue** queue =
      new modcncy::ConcurrentTaskQueue*[num_threads];
  for (size_t i = 0; i < num_threads; ++i)
    queue[i] = modcncy::ConcurrentTaskQueue::Create(
        modcncy::ConcurrentTaskQueueType::kBlockingTaskQueue);

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, begin, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  wait_policy, barrier, queue));
  }
  thread_work(begin, /*thread_index=*/0, num_threads, num_segments,
              segment_size, wait_policy, barrier, queue);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  for (size_t i = 0; i < num_threads; ++i) delete queue[i];
  delete[] queue;
  delete barrier;
}

// =============================================================================
// Parallel non-blocking segmented bitonicsort plus task stealing.
template <typename Iterator>
void waitfree(Iterator begin, Iterator end, size_t num_threads,
              size_t segment_size) {
  // Setup.
  const size_t data_size = end - begin;
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](Iterator begin, size_t thread_index, size_t num_threads,
                        size_t num_segments, size_t segment_size,
                        std::atomic<size_t>* segment_stage_count,
                        std::atomic<size_t>* thread_stage_count,
                        modcncy::ConcurrentTaskQueue** queue) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;

    auto execute_tasks = [&](size_t thread_index) {
      for (;;) {
        std::function<void()> task = std::move(queue[thread_index]->Pop());
        if (task == nullptr) break;
        task();
      }
    };  // function execute_tasks

    auto steal_tasks = [&](size_t stealer_index) {
      const size_t stealer_stage =
          thread_stage_count[stealer_index].load(std::memory_order_relaxed);
      for (size_t i = stealer_index + 1; i < num_threads + stealer_index; ++i)
        if (stealer_stage >
            thread_stage_count[i % num_threads].load(std::memory_order_relaxed))
          execute_tasks(/*thread_index=*/i % num_threads);
    };  // function steal_tasks

    for (size_t i = low_index; i < high_index; i += segment_size) {
      queue[thread_index]->Push([&, i] {
        // Sort each indiviual segment.
        std::sort(begin + i, begin + i + segment_size);
        // Mark segment "ready" for next stage.
        segment_stage_count[/*segment_id=*/i / segment_size].fetch_add(1);
      });
    }
    execute_tasks(thread_index);
    steal_tasks(thread_index);

    // Mark this thread "ready" for next stage.
    thread_stage_count[thread_index].fetch_add(1, std::memory_order_relaxed);

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
            while (thread_stage_count[thread_index].load(
                       std::memory_order_relaxed) !=
                   segment_stage_count[segment1_id].load())
              steal_tasks(thread_index);
            while (thread_stage_count[thread_index].load(
                       std::memory_order_relaxed) !=
                   segment_stage_count[segment2_id].load())
              steal_tasks(thread_index);

            if ((i & k) == 0) {
              queue[thread_index]->Push(
                  [begin, segment_stage_count, i, ij, segment1_id, segment2_id,
                   segment1_index, segment2_index, segment_size] {
                    std::atomic_thread_fence(std::memory_order_acquire);
                    typedef typename std::iterator_traits<Iterator>::value_type
                        value_type;
                    value_type* buffer = new value_type[2 * segment_size];
                    merge::Up(/*segment1=*/&*(begin + segment1_index),
                              /*segment2=*/&*(begin + segment2_index),
                              /*buffer=*/buffer,
                              /*segment_size=*/segment_size);
                    delete[] buffer;
                    // Mark segments "ready" for next stage.
                    segment_stage_count[segment1_id].fetch_add(1);
                    segment_stage_count[segment2_id].fetch_add(1);
                  });
            } else {
              queue[thread_index]->Push(
                  [begin, segment_stage_count, i, ij, segment1_id, segment2_id,
                   segment1_index, segment2_index, segment_size] {
                    std::atomic_thread_fence(std::memory_order_acquire);
                    typedef typename std::iterator_traits<Iterator>::value_type
                        value_type;
                    value_type* buffer = new value_type[2 * segment_size];
                    merge::Dn(/*segment1=*/&*(begin + segment1_index),
                              /*segment2=*/&*(begin + segment2_index),
                              /*buffer=*/buffer,
                              /*segment_size=*/segment_size);
                    delete[] buffer;
                    // Mark segments "ready" for next stage.
                    segment_stage_count[segment1_id].fetch_add(1);
                    segment_stage_count[segment2_id].fetch_add(1);
                  });
            }
          }
        }
        execute_tasks(thread_index);
        steal_tasks(thread_index);

        // Mark this thread "ready" for next stage.
        thread_stage_count[thread_index].fetch_add(1,
                                                   std::memory_order_relaxed);
      }
    }
  };  // function thread_work

  std::atomic<size_t>* segment_stage_count =
      new std::atomic<size_t>[num_segments];
  for (size_t i = 0; i < num_segments; ++i) segment_stage_count[i] = 0;

  std::atomic<size_t>* thread_stage_count =
      new std::atomic<size_t>[num_threads];
  for (size_t i = 0; i < num_threads; ++i) thread_stage_count[i] = 0;

  modcncy::ConcurrentTaskQueue** queue =
      new modcncy::ConcurrentTaskQueue*[num_threads];
  for (size_t i = 0; i < num_threads; ++i)
    queue[i] = modcncy::ConcurrentTaskQueue::Create(
        modcncy::ConcurrentTaskQueueType::kBlockingTaskQueue);

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(
        thread_work, begin, /*thread_index=*/i, num_threads, num_segments,
        segment_size, segment_stage_count, thread_stage_count, queue));
  }
  thread_work(begin, /*thread_index=*/0, num_threads, num_segments,
              segment_size, segment_stage_count, thread_stage_count, queue);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  for (size_t i = 0; i < num_threads; ++i) delete queue[i];
  delete[] queue;
  delete[] segment_stage_count;
  delete[] thread_stage_count;
}

}  // namespace bitonicsort
}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_BITONICSORT_H_
