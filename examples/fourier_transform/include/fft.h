// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// This is a series of implementations of different versions of the so-called
// Fast Fourier Transform algorithm for shared-memory computer architectures.
//
// These implementations are based on `butterfly` operations on data segments,
// except the original algorithm which is based on `butterfly` operations on
// individual data elements.
//
// Initially, for the segmented implementations, local FFTs are computed in
// individual data segments. After that, following the Cooley-Tukey data-flow
// diagram, the local FFTs are combined in order to generate the resulting FFT.
//
// There are different versions of the algorithm.
//
//   + A recursive implementation of the original Cooley-Tukey FFT, which is
//     based on `butterfly` operations on individual data elements.
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

#ifndef EXAMPLES_FOURIER_TRANSFORM_INCLUDE_FFT_H_
#define EXAMPLES_FOURIER_TRANSFORM_INCLUDE_FFT_H_

#include <modcncy/barrier.h>
#include <modcncy/wait_policy.h>

#include <atomic>
#include <cmath>
#include <complex>
#include <cstring>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

namespace fourier_transform {
namespace fft {

static constexpr float kPi = 3.14159265359;

static inline void butterfly(std::complex<float>* segment1,
                             std::complex<float>* segment2,
                             std::complex<float> twiddle_factor,
                             size_t segment_size) {
  // Decimation in frequency butterfly operation:
  // A_out = A + B
  // B_out = W * (A - B)
  std::complex<float>* out1 = new std::complex<float>[segment_size];
  std::complex<float>* out2 = new std::complex<float>[segment_size];
  for (size_t i = 0; i < segment_size; ++i) {
    out1[i] = segment1[i] + segment2[i];
    out2[i] = twiddle_factor * (segment1[i] - segment2[i]);
  }

  // Scatter back results to original arrays.
  const size_t bytes = segment_size * sizeof(std::complex<float>);
  std::memcpy(/*dst=*/segment1, /*src=*/out1, /*cnt=*/bytes);
  std::memcpy(/*dst=*/segment2, /*src=*/out2, /*cnt=*/bytes);
  delete[] out1;
  delete[] out2;
}

// =============================================================================
// Original Recursive Cooley-Tukey FFT.
void original(std::complex<float>* data, size_t data_size) {
  auto fft_rec = [](std::complex<float>* data, size_t data_size) {
    // Setup.
    if (data_size <= 1) return;

    std::complex<float>* even = new std::complex<float>[data_size / 2];
    std::complex<float>* odd = new std::complex<float>[data_size / 2];
    for (size_t i = 0; i < data_size / 2; ++i) {
      even[i] = data[i * 2];
      odd[i] = data[i * 2 + 1];
    }

    // Recursive butterfly data-flow diagram.
    original(even, data_size / 2);
    original(odd, data_size / 2);

    // Butterfly operation.
    for (size_t k = 0; k < data_size / 2; ++k) {
      std::complex<float> W =
          std::exp(std::complex<float>(0, -2 * kPi * k / data_size));
      data[k] = even[k] + odd[k];
      data[data_size / 2 + k] = W * (even[k] - odd[k]);
    }

    delete[] even;
    delete[] odd;
  };

  fft_rec(data, data_size);

  // Normalize output.
  for (size_t i = 0; i < data_size; ++i) data[i] /= data_size;
}

// =============================================================================
// Parallel pthreads segmented FFT.
void blocking(std::complex<float>* data, size_t data_size, size_t num_threads,
              size_t segment_size,
              std::function<void()> wait_policy = &modcncy::cpu_yield) {
  // Setup.
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](std::complex<float>* data, size_t thread_index,
                        size_t num_threads, size_t num_segments,
                        size_t segment_size, std::function<void()> wait_policy,
                        modcncy::Barrier* barrier) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;

    // Local FFT of each indiviual segment.
    for (size_t i = low_index; i < high_index; i += segment_size)
      fft::original(data + i, segment_size);

    barrier->Wait(num_threads, wait_policy);

    // Butterfly network.
    size_t stage_multiplier = 1;
    for (size_t j = num_segments >> 1; j > 0; j >>= 1) {
      for (size_t i = low_segment; i < high_segment; ++i) {
        const size_t ij = i ^ j;
        if (i < ij) {
          const size_t W = (i * stage_multiplier) % num_segments;
          butterfly(/*segment1=*/&data[i * segment_size],
                    /*segment2=*/&data[ij * segment_size],
                    /*twiddle_factor=*/W,
                    /*segment_size=*/segment_size);
        }
      }
      stage_multiplier <<= 1;
      barrier->Wait(num_threads, wait_policy);
    }
  };  // function thread_work

  modcncy::Barrier* barrier = modcncy::Barrier::Create(
      modcncy::BarrierType::kCentralSenseCounterBarrier);

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, data, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  wait_policy, barrier));
  }
  thread_work(data, /*thread_index=*/0, num_threads, num_segments, segment_size,
              wait_policy, barrier);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  delete barrier;
}

// =============================================================================
// Parallel non-blocking segmented FFT.
void lockfree(std::complex<float>* data, size_t data_size, size_t num_threads,
              size_t segment_size,
              std::function<void()> wait_policy = &modcncy::cpu_yield) {
  // Setup.
  const size_t num_segments = data_size / segment_size;

  // Work to be done per thread.
  auto thread_work = [](std::complex<float>* data, size_t thread_index,
                        size_t num_threads, size_t num_segments,
                        size_t segment_size, std::function<void()> wait_policy,
                        std::atomic<size_t>* segment_stage_count) {
    // Setup.
    const size_t num_segments_per_thread = num_segments / num_threads;
    const size_t low_segment = thread_index * num_segments_per_thread;
    const size_t high_segment = low_segment + num_segments_per_thread;
    const size_t low_index = low_segment * segment_size;
    const size_t high_index = high_segment * segment_size;
    size_t my_stage = 0;

    for (size_t i = low_index; i < high_index; i += segment_size) {
      // Local FFT of each individual segment.
      fft::original(data + i, segment_size);
      // Mark segment "ready" for next stage.
      segment_stage_count[/*segment_id=*/i / segment_size].fetch_add(1);
    }

    // Mark this thread "ready" for next stage.
    ++my_stage;

    // Butterfly network.
    size_t stage_multiplier = 1;
    for (size_t j = num_segments >> 1; j > 0; j >>= 1) {
      for (size_t i = low_segment; i < high_segment; ++i) {
        const size_t ij = i ^ j;
        if (i < ij) {
          const size_t W = (i * stage_multiplier) % num_segments;
          const size_t segment1_index = i * segment_size;
          const size_t segment2_index = ij * segment_size;
          const size_t segment1_id = segment1_index / segment_size;
          const size_t segment2_id = segment2_index / segment_size;

          // Wait until the segments I need are on my same stage.
          while (my_stage != segment_stage_count[segment1_id].load())
            wait_policy();
          while (my_stage != segment_stage_count[segment2_id].load())
            wait_policy();

          butterfly(/*segment1=*/&data[segment1_index],
                    /*segment2=*/&data[segment2_index],
                    /*twiddle_factor=*/W,
                    /*segment_size=*/segment_size);

          // Mark segments "ready" for next stage.
          segment_stage_count[segment1_id].fetch_add(1);
          segment_stage_count[segment2_id].fetch_add(1);
        }
      }
      ++my_stage;
      stage_multiplier <<= 1;
    }
  };  // function thread_work

  std::atomic<size_t>* segment_stage_count =
      new std::atomic<size_t>[num_segments];
  for (size_t i = 0; i < num_segments; ++i) segment_stage_count[i] = 0;

  // Launch threads.
  // Main thread also performs work as thread 0, so loops starts in index 1.
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (size_t i = 1; i < num_threads; ++i) {
    threads.push_back(std::thread(thread_work, data, /*thread_index=*/i,
                                  num_threads, num_segments, segment_size,
                                  wait_policy, segment_stage_count));
  }
  thread_work(data, /*thread_index=*/0, num_threads, num_segments, segment_size,
              wait_policy, segment_stage_count);

  // Join threads.
  // So main thread can acquire the last published changes of the other threads.
  for (auto& thread : threads) thread.join();
  delete[] segment_stage_count;
}

}  // namespace fft
}  // namespace fourier_transform

#endif  // EXAMPLES_FOURIER_TRANSFORM_INCLUDE_FFT_H_
