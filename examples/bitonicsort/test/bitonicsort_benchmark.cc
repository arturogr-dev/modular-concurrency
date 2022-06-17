// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

#include "examples/bitonicsort/src/bitonicsort.h"

// =============================================================================
// Helper to verify if data is sorted.
inline bool IsSorted(const std::vector<int>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
// Benchmark arguments.
void BenchmarkArgs(benchmark::internal::Benchmark* b) {
  // Try all barrier types and all wait policies for each barrier type over a
  // different number of execution threads.
  const int data_size_begin = 19;    // 2048 kB (Using integers of 4 bytes).
  const int data_size_end = 20;      // 4096 kB (Using integers of 4 bytes).
  const int segment_size_begin = 8;  //    1 kB (Using integers of 4 bytes).
  const int segment_size_end = 18;   // 1024 kB (Using integers of 4 bytes).
  for (int i = data_size_begin; i <= data_size_end; ++i)
    for (int j = segment_size_begin; j <= segment_size_end; ++j)
      b->Args({/*data_size=*/i, /*segment_size=*/j});
}

// =============================================================================
// Benchmark: Sequential Bitonicsort by Segments.
void BM_Bitonicsort(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const int data_size = 1 << state.range(0);
  const int segment_size = 1 << state.range(1);
  std::vector<int> data;
  data.reserve(data_size);
  for (int i = 1; i <= data_size; ++i) data.push_back(i);
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);
  assert(!IsSorted(data) && "Data should not be sorted");

  // Benchmark.
  for (auto _ : state) {
    bitonicsort::sort(data.begin(), data.end(), segment_size);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(IsSorted(data) && "Data should be sorted");
    std::shuffle(data.begin(), data.end(), gen);
    assert(!IsSorted(data) && "Data should not be sorted");
    state.ResumeTiming();
  }

  // Teardown.
  const int data_bytes = data_size * sizeof(int);
  const int segment_bytes = segment_size * sizeof(int);
  const std::string label =
      std::to_string(data_bytes / 1024) + " kB data | " +
      std::to_string(segment_bytes / 1024) + " kB segment | " +
      std::to_string(data_bytes / segment_bytes) + " segments";
  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * data_bytes);
}
BENCHMARK(BM_Bitonicsort)
    ->Apply(BenchmarkArgs)
    ->ArgNames({"data_size", "segment_size"})
    ->Unit(benchmark::kMillisecond);
