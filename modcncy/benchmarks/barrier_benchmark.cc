// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>
#include <modcncy/barrier.h>

#include <thread>

// =============================================================================
// Benchmark: Barrier Synchronization Primitive.
template <modcncy::BarrierType barrier_type>
static void BM_Barrier(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const auto& num_threads = state.threads();
  static modcncy::Barrier* barrier = nullptr;
  if (state.thread_index() == 0) {
    barrier = modcncy::Barrier::Create(barrier_type);
  }
  // Benchmark.
  for (auto _ : state) {
    barrier->Wait(num_threads);
  }
  // Teardown.
  if (state.thread_index() == 0) {
    state.SetItemsProcessed(state.iterations());
    delete barrier;
  }
}

BENCHMARK_TEMPLATE(BM_Barrier,
                   modcncy::BarrierType::kCentralSenseCounterBarrier)
    ->ThreadRange(1, std::thread::hardware_concurrency())
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_Barrier, modcncy::BarrierType::kCentralStepCounterBarrier)
    ->ThreadRange(1, std::thread::hardware_concurrency())
    ->UseRealTime();
