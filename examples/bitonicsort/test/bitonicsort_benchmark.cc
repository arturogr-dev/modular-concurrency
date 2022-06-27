// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

#include "examples/bitonicsort/include/bitonicsort.h"

// Experiment with data sizes that could potentially fit in L2, L3 and DRAM.
static constexpr int kDataSizeBegin = 15;  //  128 kB (Using 4 bytes ints).
static constexpr int kDataSizeEnd = 21;    // 8192 kB (Using 4 bytes ints).

// Experiment with segment sizes that could potentially fit only in L1.
static constexpr int kSegmentSizeBegin = 8;  // 1 kB (Using 4 bytes ints).
static constexpr int kSegmentSizeEnd = 10;   // 4 kB (Using 4 bytes ints).

// Experiment with the maximum available cpu cores.
static const int num_cpus = std::thread::hardware_concurrency();

// =============================================================================
// Computes the logarithm base 2 of a power of 2.
static inline int log2(int x) { return __builtin_ctz(x); }

// =============================================================================
// Helper to verify if data is sorted.
static bool IsSorted(const std::vector<int>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
// Benchmark arguments for sequential bitonicsort implementation.
static void SequentialArgs(benchmark::internal::Benchmark* b) {
  for (int i = kDataSizeBegin; i <= kDataSizeEnd; i += 2)
    for (int j = kSegmentSizeBegin; j <= kSegmentSizeEnd; ++j)
      b->Args({/*data_size=*/i, /*segment_size=*/j, /*threads=*/1});
}

// =============================================================================
// Benchmark arguments for parallel bitonicsort implementation.
static void ParallelArgs(benchmark::internal::Benchmark* b) {
  for (int i = kDataSizeBegin; i <= kDataSizeEnd; i += 2)
    for (int j = kSegmentSizeBegin; j <= kSegmentSizeEnd; ++j)
      for (int threads = 1; threads <= num_cpus; threads <<= 1)
        b->Args({/*data_size=*/i, /*segment_size=*/j, threads});
}

// =============================================================================
// Benchmark: Bitonicsort by Segments.
template <bitonicsort::ExecutionPolicy policy>
static void BM_SortVectorOfInts(
    benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const int data_size = 1 << state.range(0);
  const int segment_size = 1 << state.range(1);
  const int threads = state.range(2);
  std::vector<int> data;
  data.reserve(data_size);
  for (int i = 1; i <= data_size; ++i) data.push_back(i);
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);
  assert(!IsSorted(data) && "Data should not be sorted");

  // Benchmark.
  for (auto _ : state) {
    bitonicsort::sort(data.begin(), data.end(), segment_size, policy, threads);

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
      std::to_string(bitonic_stages) + " bitonic stages";
  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * data_in_bytes);
}

BENCHMARK_TEMPLATE(BM_SortVectorOfInts,
                   bitonicsort::ExecutionPolicy::kSequential)
    ->Apply(SequentialArgs)
    ->ArgNames({"data_size", "segment_size", "threads"})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_SortVectorOfInts, bitonicsort::ExecutionPolicy::kOmpBased)
    ->Apply(ParallelArgs)
    ->ArgNames({"data_size", "segment_size", "threads"})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
