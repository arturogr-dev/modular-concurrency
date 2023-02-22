// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Fourier Transform algorithms benchmarks.
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--input_shift=15\ --segment_size=1024
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> data_size = 1 << 15 = [elements] = 128 [kB]
//     -> segment_size = 1024 [elements] =  4096 [bytes]
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--input_shift=22\ --segment_size=2048
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> data_size = 1 << 22 = 4194304 [elements] = 16384 [kB]
//     -> segment_size = 2048 [elements] =  8192 [bytes]
//
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>
#include <modcncy/wait_policy.h>

#include <cmath>
#include <complex>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/fourier_transform/include/algorithm.h"
#include "examples/fourier_transform/test/fourier_transform_init.h"

namespace fourier_transform {

// Size of the input vector to be sorted by shifting this value. This represents
// the number of shifts to be performed to generate the input data size.
// Ensuring an input size of power of 2 for bitonicsort.
MODCNCY_DEFINE_int32(input_shift, 15);

// Number of elements in a segment. Thus, the size of individual smaller sorts.
MODCNCY_DEFINE_int32(segment_size, 1024);

// Number of threads to be launched for the different parallel implementations.
MODCNCY_DEFINE_int32(num_threads, std::thread::hardware_concurrency());

// Waiting policy for threads spinning at a barrier synchronization primitive.
MODCNCY_DEFINE_string(wait_policy, "cpu_yield");

namespace {

// =============================================================================
// Verifies if a sequential implementation is executed.
bool is_sequential(FftType fft_type) {
  return fft_type == FftType::kSequentialOriginalFft;
}

// =============================================================================
// Computes the logarithm base 2 of a power of 2.
size_t log2(size_t x) { return __builtin_ctz(x); }

// =============================================================================
std::function<void()> GetWaitPolicy(const std::string& policy) {
  if (policy == "cpu_no_op") return &modcncy::cpu_no_op;
  if (policy == "cpu_yield") return &modcncy::cpu_yield;
  if (policy == "cpu_pause") return &modcncy::cpu_pause;
  return &modcncy::cpu_yield;
}

// =============================================================================
std::vector<std::complex<float>> ComputeSinusoid(size_t size) {
  std::vector<std::complex<float>> data(size);
  for (size_t i = 0; i < size; ++i) data[i] = std::sin(2 * fft::kPi * i / size);
  return data;
}

// =============================================================================
// Benchmark: Fourier Transform algorithm.
template <FftType fft_type>
void BM_Fft(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const size_t data_size = 1 << FLAGS_input_shift;
  const size_t segment_size = FLAGS_segment_size;
  const size_t num_segments = data_size / segment_size;
  const size_t num_threads = is_sequential(fft_type) ? 1 : FLAGS_num_threads;
  std::function<void()> wait_policy = GetWaitPolicy(FLAGS_wait_policy);
  std::vector<std::complex<float>> data = ComputeSinusoid(data_size);

  // Benchmark.
  for (auto _ : state) {
    FFT(&data[0], data_size, fft_type, num_threads, segment_size, wait_policy);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(data.size() == data_size);
    data = ComputeSinusoid(data_size);
    state.ResumeTiming();
  }

  // Teardown.
  const size_t bytes = sizeof(std::complex<float>);
  state.SetLabel(std::to_string(data_size * bytes / 1024) + " [kB] data | " +
                 std::to_string(segment_size * bytes) + " [bytes] segment | " +
                 std::to_string(num_segments) + " num_segments | " +
                 std::to_string(num_threads) + " num_threads | " +
                 std::to_string(log2(num_segments)) + " algorithm-stages | " +
                 FLAGS_wait_policy + " wait-policy");
  state.SetBytesProcessed(state.iterations() * data_size * bytes);
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_Fft, FftType::kSequentialOriginalFft)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Fft, FftType::kParallelBlockingFft)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Fft, FftType::kParallelLockFreeFft)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

}  // namespace
}  // namespace fourier_transform

// Run benchmarks.
int main(int argc, char** argv) {
  fourier_transform::ParseCommandLineFlags(&argc, argv);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
