// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>
#include <modcncy/barrier.h>

#include <array>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

namespace barrier_type {
// Supported Barrier Types:
//   + 0: modcncy::BarrierType::kCentralSenseCounterBarrier.
//   + 1: modcncy::BarrierType::kCentralStepCounterBarrier.
static constexpr modcncy::BarrierType kCentralSenseCounterBarrier =
    modcncy::BarrierType::kCentralSenseCounterBarrier;
static constexpr modcncy::BarrierType kCentralStepCounterBarrier =
    modcncy::BarrierType::kCentralStepCounterBarrier;
// Benchmark arguments definition requires a `std::vector` of `int64_t` types:
// `Benchmark* Args(const std::vector<int64_t>& args);`
static constexpr std::array<int64_t, 2> kBarrierTypes = {
    static_cast<int64_t>(kCentralSenseCounterBarrier),
    static_cast<int64_t>(kCentralStepCounterBarrier)};
}  // namespace barrier_type

namespace wait_policy {
// Supported Wait Policies:
//   + 0: modcncy::WaitPolicy::kActiveWaiting
//   + 1: modcncy::WaitPolicy::kPassiveWaiting
//   + 2: modcncy::WaitPolicy::kPausedWaiting
static constexpr modcncy::WaitPolicy kActiveWaiting =
    modcncy::WaitPolicy::kActiveWaiting;
static constexpr modcncy::WaitPolicy kPassiveWaiting =
    modcncy::WaitPolicy::kPassiveWaiting;
static constexpr modcncy::WaitPolicy kPausedWaiting =
    modcncy::WaitPolicy::kPausedWaiting;
// Benchmark arguments definition requires a `std::vector` of `int64_t` types:
// `Benchmark* Args(const std::vector<int64_t>& args);`
static constexpr std::array<int64_t, 3> kWaitPolicies = {
    static_cast<int64_t>(kActiveWaiting), static_cast<int64_t>(kPassiveWaiting),
    static_cast<int64_t>(kPausedWaiting)};
}  // namespace wait_policy

// =============================================================================
// Barrier type label.
std::string type_label(modcncy::BarrierType type) {
  using namespace barrier_type;  // NOLINT(build/namespaces)
  if (type == kCentralSenseCounterBarrier) return "kCentralSenseCounterBarrier";
  if (type == kCentralStepCounterBarrier) return "kCentralStepCounterBarrier";
  return "42";
}

// =============================================================================
// Waiting policy label.
std::string policy_label(modcncy::WaitPolicy policy) {
  using namespace wait_policy;  // NOLINT(build/namespaces)
  if (policy == kActiveWaiting) return "kActiveWaiting";
  if (policy == kPassiveWaiting) return "kPassiveWaiting";
  if (policy == kPausedWaiting) return "kPausedWaiting";
  return "42";
}

// =============================================================================
// Benchmark label.
std::string label(modcncy::BarrierType type, modcncy::WaitPolicy policy) {
  return type_label(type) + " | " + policy_label(policy);
}

// =============================================================================
// `BM_Barrier` benchmark arguments.
void BarrierArgs(benchmark::internal::Benchmark* b) {
  // Try all barrier types and all wait policies for each barrier type over a
  // different number of execution threads.
  const int cpus = std::thread::hardware_concurrency();
  for (const auto type : barrier_type::kBarrierTypes)
    for (const auto policy : wait_policy::kWaitPolicies)
      for (int threads = 1; threads <= cpus; threads *= 2)
        b->Args({type, policy, threads});
}

// =============================================================================
// Benchmark: Barrier Synchronization Primitive.
// This benchmark internally launches multiple threads to go through the barrier
// multiple times. As contention degrades performance, the overall time to go
// through the barrier should increase as the number of threads increases. Use
// this benchmark to measure how long it takes a thread to go through the
// barrier as the number of threads increases. The wall time is expected to
// increase as the number of threads increases.
void BM_Barrier(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const auto type = static_cast<modcncy::BarrierType>(state.range(0));
  const auto policy = static_cast<modcncy::WaitPolicy>(state.range(1));
  const auto num_threads = static_cast<int>(state.range(2));
  auto barrier = modcncy::Barrier::Create(type);
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  // Benchmark.
  for (auto _ : state) {
    // Launch `num_threads - 1` additional threads, but pause the timer to avoid
    // measuring the overhead of creating the additional threads.
    state.PauseTiming();
    for (int thread_index = 1; thread_index < num_threads; ++thread_index)
      threads.emplace_back([&] { barrier->Wait(num_threads, policy); });
    state.ResumeTiming();
    // Main thread also contends. Measure the time it takes to the main thread
    // to go through the barrier.
    barrier->Wait(num_threads, policy);
    // Prepare for next benchmark iteration, but pause the timer to avoid
    // measuring the overhead of joining the additional threads.
    state.PauseTiming();
    for (auto& thread : threads) thread.join();
    threads.clear();
    state.ResumeTiming();
  }

  // Teardown.
  state.SetItemsProcessed(state.iterations());
  state.SetLabel(label(type, policy));
  delete barrier;
}
BENCHMARK(BM_Barrier)
    ->Apply(BarrierArgs)
    ->ArgNames({"type", "policy", "threads"})
    ->UseRealTime();
