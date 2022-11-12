// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Sorting algorithms benchmarks.
//
// + Example usage:
//
//   $ make benchmark benchmark_args=--input_shift=15\ --segment_size=1024
//
//   It will test the following scenario, assuming 32-bit integers:
//
//     -> data_size = 1 << 15 = [elements] = [kB]
//     -> segment_size = 1024 [elements] =  [bytes]
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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/sorting/include/algorithm.h"
#include "examples/sorting/test/sorting_init.h"

namespace sorting {

// Size of the input vector to be sorted by shifting this value. This represents
// the number of shifts to be performed to generate the input data size.
// Ensuring an input size of power of 2 for bitonicsort.
MODCNCY_DEFINE_int32(input_shift, 22);

// Number of elements in a segment. Thus, the size of individual smaller sorts.
MODCNCY_DEFINE_int32(segment_size, 1024);

// Number of threads to be launched for the different parallel implementations.
MODCNCY_DEFINE_int32(num_threads, std::thread::hardware_concurrency());

// Waiting policy for threads spinning at a barrier synchronization primitive.
MODCNCY_DEFINE_string(wait_policy, "cpu_yield");

namespace {

// =============================================================================
// Verifies if a sequential implementation is executed.
bool is_sequential(SortType sort_type) {
  return sort_type == SortType::kSequentialStdSort ||
         sort_type == SortType::kSequentialOriginalBitonicsort ||
         sort_type == SortType::kSequentialSegmentedBitonicsort;
}

// =============================================================================
// Verifies if a bitonicsort implementation is executed.
bool is_bitonicsort(SortType sort_type) {
  return sort_type == SortType::kSequentialOriginalBitonicsort ||
         sort_type == SortType::kSequentialSegmentedBitonicsort ||
         sort_type == SortType::kParallelOmpBasedBitonicsort ||
         sort_type == SortType::kParallelBlockingBitonicsort ||
         sort_type == SortType::kParallelLockFreeBitonicsort ||
         sort_type == SortType::kParallelStealingBitonicsort ||
         sort_type == SortType::kParallelWaitFreeBitonicsort;
}

// =============================================================================
// Computes the logarithm base 2 of a power of 2.
size_t log2(size_t x) { return __builtin_ctz(x); }

// =============================================================================
// Returns the number of stages/phases in the algorithm.
std::string algorithm_stages_label(size_t num_segments, SortType sort_type) {
  if (is_bitonicsort(sort_type))
    return std::to_string((log2(num_segments) * (log2(num_segments) + 1)) / 2);
  return "N/A";  // TODO(arturogr-dev): Check others to get this right.
}

// =============================================================================
// Returns the applied wait policy.
std::string wait_policy_label(const std::string& policy, SortType sort_type) {
  if (is_bitonicsort(sort_type) && !is_sequential(sort_type)) {
    if (policy == "cpu_no_op" || policy == "cpu_yield" || policy == "cpu_pause")
      return policy;
    return "cpu_yield";
  }
  return "N/A";
}

// =============================================================================
// Verifies if the data is sorted in non-descending order.
template <typename T>
bool IsSorted(const std::vector<T>& data) {
  for (size_t i = 1; i < data.size(); ++i)
    if (data[i - 1] > data[i]) return false;
  return true;
}

// =============================================================================
std::function<void()> GetWaitPolicy(const std::string& policy) {
  if (policy == "cpu_no_op") return &modcncy::cpu_no_op;
  if (policy == "cpu_yield") return &modcncy::cpu_yield;
  if (policy == "cpu_pause") return &modcncy::cpu_pause;
  return &modcncy::cpu_yield;
}

// =============================================================================
// Benchmark: Sorting algorithm.
template <typename T, SortType sort_type>
void BM_Sort(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const size_t data_size = 1 << FLAGS_input_shift;
  const size_t segment_size = FLAGS_segment_size;
  const size_t num_segments = data_size / segment_size;
  const size_t num_threads = is_sequential(sort_type) ? 1 : FLAGS_num_threads;
  std::function<void()> wait_policy = GetWaitPolicy(FLAGS_wait_policy);
  std::vector<T> data(data_size);
  for (size_t i = 0; i < data_size; ++i) data[i] = static_cast<T>(i);
  std::random_device rand_dev;
  std::mt19937 rand_gen(rand_dev());
  std::shuffle(data.begin(), data.end(), rand_gen);
  assert(!IsSorted(data) && "Data should not be sorted after shuffle");

  // Benchmark.
  for (auto _ : state) {
    sort(data.begin(), data.end(), sort_type, num_threads, segment_size,
         wait_policy);

    // Prepare for next iteration.
    state.PauseTiming();
    assert(IsSorted(data) && "Data should be sorted");
    std::shuffle(data.begin(), data.end(), rand_gen);
    assert(!IsSorted(data) && "Data should not be sorted after shuffle");
    state.ResumeTiming();
  }

  // Teardown.
  state.SetLabel(
      std::to_string(data_size * sizeof(T) / 1024) + " [kB] data | " +
      std::to_string(segment_size * sizeof(T)) + " [bytes] segment | " +
      std::to_string(num_segments) + " num_segments | " +
      std::to_string(num_threads) + " num_threads | " +
      algorithm_stages_label(num_segments, sort_type) + " algorithm-stages | " +
      wait_policy_label(FLAGS_wait_policy, sort_type) + " wait-policy");
  state.SetBytesProcessed(state.iterations() * data_size * sizeof(T));
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kSequentialStdSort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kSequentialOriginalBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kSequentialSegmentedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelOmpBasedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelBlockingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelLockFreeBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelStealingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelWaitFreeBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelGnuMultiwayMergesort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelGnuQuicksort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int32_t, SortType::kParallelGnuBalancedQuicksort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kSequentialStdSort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kSequentialOriginalBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kSequentialSegmentedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelOmpBasedBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelBlockingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelLockFreeBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelStealingBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelWaitFreeBitonicsort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelGnuMultiwayMergesort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelGnuQuicksort)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Sort, int64_t, SortType::kParallelGnuBalancedQuicksort)
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
