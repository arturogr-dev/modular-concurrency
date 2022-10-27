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
template <sorting::Type type>
static void BM_Sort32BitInts(benchmark::State& state) {  // NOLINT
  // Setup.
  const size_t data_size = 1 << state.range(0);  // Ensures a power of 2.
  const size_t segment_size = 2048;              // Elements.
  size_t num_threads = std::thread::hardware_concurrency();
  if (type == sorting::Type::kStdSort ||
      type == sorting::Type::kOriginalBitonicsort ||
      type == sorting::Type::kSegmentedBitonicsort)
    num_threads = 1;
  std::vector<int32_t> data;
  data.reserve(data_size);
  for (size_t i = 0; i < data_size; ++i)
    data.push_back(static_cast<int32_t>(i));
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::shuffle(data.begin(), data.end(), gen);
  assert(!IsSorted(data) && "Data should not be sorted");

  // Benchmark.
  for (auto _ : state) {
    sorting::sort(data.begin(), data.end(), type, num_threads, segment_size);

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
      std::to_string(num_threads) + " threads ";
  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * data_in_bytes);
}

BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kStdSort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kOriginalBitonicsort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kSegmentedBitonicsort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kOmpBasedBitonicsort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kNonBlockingBitonicsort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort32BitInts, sorting::Type::kGnuMultiwayMergesort)
    ->DenseRange(kDataSizeBegin, kDataSizeEnd, kDataSizeStep)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

BENCHMARK_MAIN();
