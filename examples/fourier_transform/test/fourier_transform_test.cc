// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "examples/fourier_transform/include/algorithm.h"

namespace fourier_transform {
namespace {

class FourierTransformTest : public testing::TestWithParam<FftType> {};

INSTANTIATE_TEST_SUITE_P(AllFftTypes, FourierTransformTest,
                         testing::Values(FftType::kSequentialOriginalFft,
                                         FftType::kParallelBlockingFft,
                                         FftType::kParallelLockFreeFft));

// =============================================================================
std::vector<std::complex<float>> ComputeSinusoid(size_t size) {
  std::vector<std::complex<float>> data(size);
  for (size_t i = 0; i < size; ++i) data[i] = std::sin(2 * fft::kPi * i / size);
  return data;
}

// =============================================================================
size_t NumberOfErrors(const std::vector<std::complex<float>>& data,
                      const std::vector<std::complex<float>>& expected,
                      float epsilon) {
  size_t errors = 0;
  for (size_t i = 0; i < data.size(); ++i) {
    const float data_real = std::fabs(data[i].real());
    const float data_imag = std::fabs(data[i].imag());
    const float expected_real = std::fabs(expected[i].real());
    const float expected_imag = std::fabs(expected[i].imag());
    const float diff_real = std::fabs(data_real - expected_real);
    const float diff_imag = std::fabs(data_imag - expected_imag);
    if (diff_real > epsilon || diff_imag > epsilon) ++errors;
  }
  return errors;
}

// =============================================================================
TEST_P(FourierTransformTest, SmallFftCorrectnessTest) {
  constexpr size_t size = 2048;
  std::vector<std::complex<float>> data = ComputeSinusoid(size);

  std::vector<std::complex<float>> expected_fft(data.begin(), data.end());
  FFT(&expected_fft[0], size, FftType::kSequentialOriginalFft);

  FFT(&data[0], size, /*fft_type=*/GetParam(), /*num_threads=*/4,
      /*segment_size=*/256);

  constexpr float epsilon = 0.1;
  constexpr float error_rate = 0.1;
  EXPECT_LE(NumberOfErrors(data, expected_fft, epsilon), size * error_rate);
}

// =============================================================================
TEST_P(FourierTransformTest, LargeFftCorrectnessTest) {
  constexpr size_t size = 1 << 22;
  std::vector<std::complex<float>> data = ComputeSinusoid(size);

  std::vector<std::complex<float>> expected_fft(data.begin(), data.end());
  FFT(&expected_fft[0], size, FftType::kSequentialOriginalFft);

  FFT(&data[0], size, /*fft_type=*/GetParam(), /*num_threads=*/4,
      /*segment_size=*/1 << 12);

  constexpr float epsilon = 0.000001;
  constexpr float error_rate = 0.001;
  EXPECT_LE(NumberOfErrors(data, expected_fft, epsilon), size * error_rate);
}

}  // namespace
}  // namespace fourier_transform
