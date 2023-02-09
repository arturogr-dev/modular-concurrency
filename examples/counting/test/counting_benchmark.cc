// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Counting implementations benchmarks.
//
// + Example usage:
//
//   $ make benchmark
//
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <cassert>
#include <cstddef>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/counting/include/algorithm.h"
#include "examples/counting/test/counting_init.h"

namespace counting {

// Number of increments that each thread needs to perform.
MODCNCY_DEFINE_int32(increments_per_thread, 10000);

// Maximum number of threads to perform increments.
MODCNCY_DEFINE_int32(max_num_threads, 1024);

namespace {

// =============================================================================
// Benchmark: Counting.
template <CounterType counter_type>
void BM_Counting(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const size_t num_threads = state.range(0);
  const size_t items_processed = FLAGS_increments_per_thread * num_threads;
  Counter* counter = Counter::Create(counter_type);
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  // Benchmark.
  for (auto _ : state) {
    for (size_t i = 0; i < num_threads; ++i) {
      threads.emplace_back([&] {
        for (int32_t j = 0; j < FLAGS_increments_per_thread; ++j)
          counter->Increment();
      });
    }
    for (auto& thread : threads) thread.join();
    assert(counter->Count() == items_processed);
    counter->Reset();
    threads.clear();
  }

  // Teardown.
  state.SetItemsProcessed(state.iterations() * items_processed);
  delete counter;
}

// Register benchmarks.
BENCHMARK_TEMPLATE(BM_Counting, CounterType::kAtomicCounter)
    ->RangeMultiplier(2)
    ->Range(1, FLAGS_max_num_threads)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

}  // namespace
}  // namespace counting

// Run benchmarks.
int main(int argc, char** argv) {
  counting::ParseCommandLineFlags(&argc, argv);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
