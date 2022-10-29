// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Sorting algorithms benchmarks.
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--data_shift=23\ --segment_size=2048
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> data_size = 1 << 22 = 4194304 [elements] = 16384 [kB]
//     -> segment_size = 2048 [elements] =  8192 [bytes]
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--data_shift=15\ --segment_size=1024
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> data_size = 1 << 15 = [elements] = [kB]
//     -> segment_size = 1024 [elements] =  [bytes]
//
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/sorting/include/sorting.h"
#include "examples/sorting/test/sorting_init.h"

namespace sorting {

// Size of the input vector to be sorted by shifting this value. This represents
// the number of shifts to be performed to generate the input data size.
// Ensuring an input size of power of 2 for bitonicsort.
MODCNCY_DEFINE_int32(data_shift, 22);

// Number of elements in a segment. Thus, the size of individual smaller sorts.
MODCNCY_DEFINE_int32(segment_size, 2048);

// Number of threads to be launched for the different parallel implementations.
MODCNCY_DEFINE_int32(num_threads, std::thread::hardware_concurrency());

namespace {

// =============================================================================
// Computes the logarithm base 2 of a power of 2.
size_t log2(size_t x) { return __builtin_ctz(x); }

// =============================================================================
// Verifies if the data is sorted.
template <typename data_type>
bool is_sorted(const std::vector<data_type>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
// Verifies if a sequential implementation is executed.
bool is_sequential(SortType sort_type) {
  return sort_type == SortType::kStdSort ||
         sort_type == SortType::kOriginalBitonicsort ||
         sort_type == SortType::kSegmentedBitonicsort;
}

// =============================================================================
// Returns the number of times a thread hits a barrier synchronization point.
size_t get_barrier_stages(size_t num_segments, SortType sort_type) {
  switch (sort_type) {
    case SortType::kStdSort:
    case SortType::kOriginalBitonicsort:
    case SortType::kSegmentedBitonicsort:
      return 0;
    case SortType::kOmpBasedBitonicsort:
    case SortType::kNonBlockingBitonicsort:
      return (log2(num_segments) * (log2(num_segments) + 1)) / 2;
    case SortType::kGnuMultiwayMergesort:
      return 0;  // TODO(arturogr-dev): Check implementation to get this right.
  }
  return 0;
}

// =============================================================================
// Returns the label to be printed in each benchmark.
template <typename data_type>
std::string get_label(size_t data_size, size_t segment_size, size_t num_threads,
                      SortType sort_type) {
  const size_t data_in_bytes = data_size * sizeof(data_type);
  const size_t segment_in_bytes = segment_size * sizeof(data_type);
  const size_t num_segments = data_size / segment_size;
  const int barrier_stages = get_barrier_stages(num_segments, sort_type);
  return std::to_string(data_in_bytes / 1024) + " [kB] data | " +
         std::to_string(segment_in_bytes) + " [bytes] segment | " +
         std::to_string(num_segments) + " segments | " +
         std::to_string(barrier_stages) + " barrier stages | " +
         std::to_string(num_threads) + " threads ";
}

// =============================================================================
// Benchmark: Sorting algorithm.
template <typename data_type, SortType sort_type>
void BM_Sort(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const size_t data_size = 1 << FLAGS_data_shift;
  const size_t segment_size = FLAGS_segment_size;
  const size_t num_threads = is_sequential(sort_type) ? 1 : FLAGS_num_threads;
  std::vector<data_type> data(data_size);
  for (size_t i = 0; i < data_size; ++i) data[i] = static_cast<data_type>(i);
  std::random_device rand_dev;
  std::mt19937 rand_gen(rand_dev());
  std::shuffle(data.begin(), data.end(), rand_gen);
  assert(!is_sorted<data_type>(data) && "Data should not be sorted");

  // Benchmark.
  for (auto _ : state) {
    sort(data.begin(), data.end(), sort_type, num_threads, segment_size);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(is_sorted<data_type>(data) && "Data should be sorted");
    std::shuffle(data.begin(), data.end(), rand_gen);
    assert(!is_sorted<data_type>(data) && "Data should not be sorted");
    state.ResumeTiming();
  }

  // Teardown.
  state.SetLabel(
      get_label<data_type>(data_size, segment_size, num_threads, sort_type));
  state.SetBytesProcessed(state.iterations() * data_size * sizeof(data_type));
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kStdSort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kOriginalBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kSegmentedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kOmpBasedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kNonBlockingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kGnuMultiwayMergesort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

}  // namespace
}  // namespace sorting

// Run benchmarks.
int main(int argc, char** argv) {
  sorting::ParseCommandLineFlags(&argc, argv);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
