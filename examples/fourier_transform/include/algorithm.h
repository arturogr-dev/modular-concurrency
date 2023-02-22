// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// In-place Fast Fourier Transform implementations.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_FOURIER_TRANSFORM_INCLUDE_ALGORITHM_H_
#define EXAMPLES_FOURIER_TRANSFORM_INCLUDE_ALGORITHM_H_

#include <modcncy/wait_policy.h>

#include <complex>
#include <functional>

#include "examples/fourier_transform/include/fft.h"

namespace fourier_transform {

// Supported execution policies.
enum class FftType {
  kSequentialOriginalFft = 0,  // Sequential recursive Cooley–Tukey FFT.
  kParallelBlockingFft = 1,    // Barrier-based parallel FFT.
  kParallelLockFreeFft = 2,    // Lock-free parallel FFT.
};

// =============================================================================
// Main function to execute the different FFT algorithms.
void FFT(std::complex<float>* data, size_t data_size,
         FftType fft_type = FftType::kSequentialOriginalFft,
         size_t num_threads = std::thread::hardware_concurrency(),
         size_t segment_size = 1 /*number of elements*/,
         std::function<void()> wait_policy = &modcncy::cpu_yield) {
  switch (fft_type) {
    case FftType::kSequentialOriginalFft:
      fft::original(data, data_size);
      break;
    case FftType::kParallelBlockingFft:
      fft::blocking(data, data_size, num_threads, segment_size, wait_policy);
      break;
    case FftType::kParallelLockFreeFft:
      fft::lockfree(data, data_size, num_threads, segment_size, wait_policy);
      break;
  }
}

}  // namespace fourier_transform

#endif  // EXAMPLES_FOURIER_TRANSFORM_INCLUDE_ALGORITHM_H_
