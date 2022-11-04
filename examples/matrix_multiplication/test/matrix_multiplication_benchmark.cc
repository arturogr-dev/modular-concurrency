// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Matrix multiplication algorithms benchmarks.
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--input_shift=9\ --num_threads=4
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> each_matrix_size = 1 << 9 -> 512 * 512 [elements] = 1024 [kB]
//     -> num_threads = 4 [threads]
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--input_shift=11\ --num_threads=8
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> each_matrix_size = 1 << 11 -> 2048 * 2048 [elements] = 16384 [kB]
//     -> num_threads = 8 [threads]
//
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/matrix_multiplication/include/algorithm.h"
#include "examples/matrix_multiplication/test/matrix_multiplication_init.h"

namespace matrix_multiplication {

// Size of the input vector to be sorted by shifting this value. This represents
// the number of shifts to be performed to generate the input data size.
// Ensuring an input size of power of 2 and squared input matrics of same size.
MODCNCY_DEFINE_int32(input_shift, 9);

// Number of threads to be launched for the different parallel implementations.
MODCNCY_DEFINE_int32(num_threads, std::thread::hardware_concurrency());

namespace {

// =============================================================================
// Verifies if a sequential implementation is executed.
bool is_sequential(MultiplyType multiply_type) {
  return multiply_type == MultiplyType::kSequentialNaive ||
         multiply_type == MultiplyType::kSequentialCacheFriendly;
}

// =============================================================================
// Returns a square matrix of size `size^2`.
template <typename T>
std::vector<std::vector<T>> get_matrix(size_t size) {
  std::vector<std::vector<T>> matrix(size, std::vector<T>(size));
  std::srand(std::time(0));
  for (size_t i = 0; i < size; ++i)
    for (size_t j = 0; j < size; ++j)
      matrix[i][j] = static_cast<T>(std::rand() % 32768);
  return matrix;
}

// =============================================================================
// Returns the label to be printed in each benchmark.
template <typename T>
std::string get_label(size_t size, size_t num_threads) {
  const size_t size_in_bytes = size * size * sizeof(T);
  return std::to_string(size_in_bytes / 1024) + " [kB] each matrix | " +
         std::to_string(num_threads) + " threads";
}

// =============================================================================
// Benchmark: Matrix multiplication.
template <typename T, MultiplyType mult_type>
void BM_MatMul(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const size_t size = 1 << FLAGS_input_shift;
  const size_t num_threads = is_sequential(mult_type) ? 1 : FLAGS_num_threads;
  const std::vector<std::vector<T>> A = get_matrix<T>(size);
  const std::vector<std::vector<T>> B = get_matrix<T>(size);
  std::vector<std::vector<T>> C;

  // Benchmark.
  for (auto _ : state) {
    C = multiply(A, B, mult_type, num_threads);
  }

  // Teardown.
  assert(C == multiply(A, B, MultiplyType::kParallelCacheFriendly));
  state.SetLabel(get_label<T>(size, num_threads));
  state.SetBytesProcessed(state.iterations() * 3 * size * size * sizeof(T));
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_MatMul, int32_t, MultiplyType::kSequentialNaive)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int32_t, MultiplyType::kSequentialCacheFriendly)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int32_t, MultiplyType::kParallelNaive)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int32_t, MultiplyType::kParallelCacheFriendly)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int64_t, MultiplyType::kSequentialNaive)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int64_t, MultiplyType::kSequentialCacheFriendly)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int64_t, MultiplyType::kParallelNaive)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_MatMul, int64_t, MultiplyType::kParallelCacheFriendly)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

}  // namespace
}  // namespace matrix_multiplication

// Run benchmarks.
int main(int argc, char** argv) {
  matrix_multiplication::ParseCommandLineFlags(&argc, argv);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
