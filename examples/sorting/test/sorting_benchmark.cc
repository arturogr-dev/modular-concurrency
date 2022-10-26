// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

#include "examples/sorting/include/bitonicsort.h"

// Experiment sorting data sizes that could potentially fit in L2, L3 and DRAM.
static constexpr int kDataSizeBegin = 15;  //   128 kB (Assuming 4 bytes ints).
static constexpr int kDataSizeEnd = 23;    // 32768 kB (Assuming 4 bytes ints).
static constexpr int kDataSizeStep = 1;    // Input size incremental step.

// =============================================================================
// Helper to compute the logarithm base 2 of a power of 2.
static inline int log2(int x) { return __builtin_ctz(x); }

// =============================================================================
// Helper to verify if data is sorted.
static bool IsSorted(const std::vector<int>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
// Benchmark: Bitonicsort by Segments.
template <bitonicsort::ExecutionPolicy policy>
static void BM_Ints(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const int data_size = 1 << state.range(0);  // Ensures a power of 2.
  const int segment_size = bitonicsort::internal::kDefaultSegmentSize;
  const int threads = (policy != bitonicsort::ExecutionPolicy::kSequential)
                          ? std::thread::hardware_concurrency()
                          : 1;
  std::vector<int> data;
  data.reserve(data_size);
  for (int i = 1; i <= data_size; ++i) data.push_back(i);
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);
  assert(!IsSorted(data) && "Data should not be sorted");

  // Benchmark.
  for (auto _ : state) {
    bitonicsort::sort(data.begin(), data.end(), policy, threads, segment_size);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(IsSorted(data) && "Data should be sorted");
    std::shuffle(data.begin(), data.end(), gen);
    assert(!IsSorted(data) && "Data should not be sorted");
    state.ResumeTiming();
  }

  // Teardown.
  const int data_in_bytes = data_size * sizeof(int);
  const int segment_in_bytes = segment_size * sizeof(int);
  const int num_segments = data_size / segment_size;
  const int log_num_segments = log2(num_segments);
  const int bitonic_stages = (log_num_segments * (log_num_segments + 1)) / 2;
  const std::string label =
      std::to_string(data_in_bytes / 1024) + " kB data | " +
      std::to_string(segment_in_bytes / 1024) + " kB segment | " +
      std::to_string(num_segments) + " segments | " +
      std::to_string(bitonic_stages) + " bitonic stages | " +
      std::to_string(threads) + " threads ";
  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * data_in_bytes);
}

BENCHMARK_TEMPLATE(BM_Ints, bitonicsort::ExecutionPolicy::kSequential)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Ints, bitonicsort::ExecutionPolicy::kOmpBased)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Ints, bitonicsort::ExecutionPolicy::kNonBlocking)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Ints, bitonicsort::ExecutionPolicy::kGnuMergesort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

BENCHMARK_MAIN();
