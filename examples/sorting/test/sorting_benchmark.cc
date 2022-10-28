// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/sorting/include/sorting.h"
#include "examples/sorting/test/sorting_init.h"

namespace sorting_init {

// Define the declared command line flags in the `sorting_init` namespace.
//
//   + `data_shift` determines the size of the input vector to be sorted. This
//     represents the number of shifts to be performed to generate the input
//     data size. Ensuring an input size of power of 2 for bitonicsort.
//
//   + `segment_size` determines the number of elements in each segment. This
//     would also represent the size of each individual smaller sort.
//
//   + `num_threads` determines the number of execution threads to be launched
//     in the different parallel implementations.
//
// For example, if you run:
//
//   $ make benchmark benchmark_args=--data_shift=23\ --segment_size=2048
//
// It will test the following scenario, assuming 32-bit integers:
//
//   -> data_size = 1 << 22 = 4194304 [elements] = 16384 [kB]
//   -> segment_size = 2048 [elements] =  8192 [bytes]
//
// Another example of usage is:
//
//   $ make benchmark benchmark_args=--data_shift=15\ --segment_size=1024

MODCNCY_DEFINE_int32(data_shift, 22);
MODCNCY_DEFINE_int32(segment_size, 2048);
MODCNCY_DEFINE_int32(num_threads, std::thread::hardware_concurrency());

}  // namespace sorting_init

// =============================================================================
// Computes the logarithm base 2 of a power of 2.
static inline int log2(int x) { return __builtin_ctz(x); }

// =============================================================================
// Verifies if the data is sorted.
static bool is_sorted(const std::vector<int>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
// Verifies if a sequential implementation is executed.
static bool is_sequential(sorting::Type type) {
  return type == sorting::Type::kStdSort ||
         type == sorting::Type::kOriginalBitonicsort ||
         type == sorting::Type::kSegmentedBitonicsort;
}

// =============================================================================
// Benchmark: Sorting 32-bit integers.
template <sorting::Type type>
static void BM_Sort32BitInts(benchmark::State& state) {  // NOLINT
  // Setup.
  using namespace sorting_init;  // NOLINT(build/namespaces)
  const size_t data_size = 1 << FLAGS_data_shift;
  const size_t segment_size = FLAGS_segment_size;
  const size_t num_threads = is_sequential(type) ? 1 : FLAGS_num_threads;
  std::vector<int32_t> data;
  data.reserve(data_size);
  for (size_t i = 0; i < data_size; ++i)
    data.push_back(static_cast<int32_t>(i));
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);
  assert(!is_sorted(data) && "Data should not be sorted after shuffle");

  // Benchmark.
  for (auto _ : state) {
    sorting::sort(data.begin(), data.end(), type, num_threads, segment_size);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(is_sorted(data) && "Data should be sorted");
    std::shuffle(data.begin(), data.end(), gen);
    assert(!is_sorted(data) && "Data should not be sorted after shuffle");
    state.ResumeTiming();
  }

  // Teardown.
  const int data_in_bytes = data_size * sizeof(int);
  const int segment_in_bytes = segment_size * sizeof(int);
  const int num_segments = data_size / segment_size;
  const int log_num_segments = log2(num_segments);
  const int barrier_stages = (log_num_segments * (log_num_segments + 1)) / 2;
  const std::string label =
      std::to_string(data_in_bytes / 1024) + " [kB] data | " +
      std::to_string(segment_in_bytes) + " [bytes] segment | " +
      std::to_string(num_segments) + " segments | " +
      std::to_string(barrier_stages) + " barrier stages | " +
      std::to_string(num_threads) + " threads ";
  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * data_in_bytes);
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kStdSort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kOriginalBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kSegmentedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kOmpBasedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kNonBlockingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kGnuMultiwayMergesort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

// Run benchmarks.
int main(int argc, char** argv) {
  sorting_init::ParseCommandLineFlags(&argc, argv);
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
