// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <benchmark/benchmark.h>
#include <modcncy/barrier.h>

#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

// Supported Barrier Types:
//   + 0: modcncy::BarrierType::kCentralSenseCounterBarrier.
//   + 1: modcncy::BarrierType::kCentralStepCounterBarrier.
namespace barrier_type {
static constexpr modcncy::BarrierType kCentralSenseCounterBarrier =
    modcncy::BarrierType::kCentralSenseCounterBarrier;
static constexpr modcncy::BarrierType kCentralStepCounterBarrier =
    modcncy::BarrierType::kCentralStepCounterBarrier;

// Benchmark arguments definition requires a `std::vector` of `int64_t` types:
// `Benchmark* Args(const std::vector<int64_t>& args);`
static const std::vector<int64_t> kBarrierTypes = {
    static_cast<int64_t>(kCentralSenseCounterBarrier),
    static_cast<int64_t>(kCentralStepCounterBarrier)};
}  // namespace barrier_type

// Supported Waiting Policies:
//   + 0: modcncy::WaitingPolicy::kActiveWaiting
//   + 1: modcncy::WaitingPolicy::kPassiveWaiting
//   + 2: modcncy::WaitingPolicy::kPausedWaiting
namespace waiting_policy {
static constexpr modcncy::WaitingPolicy kActiveWaiting =
    modcncy::WaitingPolicy::kActiveWaiting;
static constexpr modcncy::WaitingPolicy kPassiveWaiting =
    modcncy::WaitingPolicy::kPassiveWaiting;
static constexpr modcncy::WaitingPolicy kPausedWaiting =
    modcncy::WaitingPolicy::kPausedWaiting;

// Benchmark arguments definition requires a `std::vector` of `int64_t` types:
// `Benchmark* Args(const std::vector<int64_t>& args);`
static const std::vector<int64_t> kWaitingPolicies = {
    static_cast<int64_t>(kActiveWaiting), static_cast<int64_t>(kPassiveWaiting),
    static_cast<int64_t>(kPausedWaiting)};
}  // namespace waiting_policy

// =============================================================================
// Barrier type label.
std::string barrier_type_label(modcncy::BarrierType type) {
  using namespace barrier_type;  // NOLINT(build/namespaces)
  if (type == kCentralSenseCounterBarrier) return "kCentralSenseCounterBarrier";
  if (type == kCentralStepCounterBarrier) return "kCentralStepCounterBarrier";
  return "42";
}

// =============================================================================
// Waiting policy label.
std::string waiting_policy_label(modcncy::WaitingPolicy policy) {
  using namespace waiting_policy;  // NOLINT(build/namespaces)
  if (policy == kActiveWaiting) return "kActiveWaiting";
  if (policy == kPassiveWaiting) return "kPassiveWaiting";
  if (policy == kPausedWaiting) return "kPausedWaiting";
  return "42";
}

// =============================================================================
// Benchmark label.
std::string bm_label(modcncy::BarrierType type, modcncy::WaitingPolicy policy,
                     int barrier_iterations) {
  return barrier_type_label(type) + " | " + waiting_policy_label(policy) +
         " | " + std::to_string(barrier_iterations) + " barrier iterations";
}

// =============================================================================
// `BM_Barrier` benchmark arguments.
void BarrierArgs(benchmark::internal::Benchmark* b) {
  // Try all barrier types and all waiting policies for each barrier type.
  for (const auto type : barrier_type::kBarrierTypes)
    for (const auto policy : waiting_policy::kWaitingPolicies)
      b->Args({type, policy});
}

// =============================================================================
// Benchmark: Barrier Synchronization Primitive.
// This is a simple benchmark to measure the overhead of a barrier primitive. It
// is known that thread contention degrades performance. Use this benchmark to
// measure how long it takes a thread to go through the barrier primitive as the
// number of threads increases.
// The cpu time is expected to increase as the number of threads increases.
void BM_Barrier(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const auto type = static_cast<modcncy::BarrierType>(state.range(0));
  const auto policy = static_cast<modcncy::WaitingPolicy>(state.range(1));
  const int num_threads = state.threads();
  static modcncy::Barrier* barrier = nullptr;
  constexpr int barrier_iterations = 1;
  if (state.thread_index() == 0) {
    barrier = modcncy::NewBarrier(type);
  }

  // Benchmark.
  for (auto _ : state) {
    barrier->Wait(num_threads, policy);
  }

  // Teardown.
  if (state.thread_index() == 0) {
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(bm_label(type, policy, barrier_iterations));
    delete barrier;
  }
}
BENCHMARK(BM_Barrier)
    ->Apply(BarrierArgs)
    ->ArgNames({"barrier_type", "waiting_policy"})
    ->ThreadRange(1, std::thread::hardware_concurrency())
    ->UseRealTime();

// =============================================================================
// `BM_BarrierIters` benchmark arguments.
void BarrierItersArgs(benchmark::internal::Benchmark* b) {
  // Try all barrier types and all waiting policies for each barrier type over
  // a different number of execution threads.
  const int num_cpus = std::thread::hardware_concurrency();
  for (const auto type : barrier_type::kBarrierTypes)
    for (const auto policy : waiting_policy::kWaitingPolicies)
      for (int num_threads = 1; num_threads <= num_cpus; num_threads *= 2)
        b->Args({type, policy, num_threads});
}

// =============================================================================
// Benchmark: Multiple Barrier Synchronization Stages.
// This benchmark internally launches multiple threads to hit a barrier a given
// number of times. As contention degrades performance, the overall time to
// complete the same number of barrier iterations should increase as the number
// of threads increases. Use this benchmark to measure how long it takes a
// thread to finish this same number of iterations as the number of threads
// increases. As opposed to the `BM_Barrier` benchmark, the goal of this
// scenario is to make the wall time a more meaningful measurement.
// The wall time is expected to increase as the number of threads increases.
void BM_BarrierIters(benchmark::State& state) {  // NOLINT(runtime/references)
  // Setup.
  const auto type = static_cast<modcncy::BarrierType>(state.range(0));
  const auto policy = static_cast<modcncy::WaitingPolicy>(state.range(1));
  const int num_threads = state.range(2);
  auto barrier = modcncy::NewBarrier(type);
  constexpr int barrier_iterations = 10000;
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  // Helper function to emulate a high contention scenario.
  auto HitBarrier = [](modcncy::Barrier* barrier, modcncy::WaitingPolicy policy,
                       int num_threads, int barrier_iterations) {
    for (int i = 0; i < barrier_iterations; ++i)
      barrier->Wait(num_threads, policy);
  };  // function HitBarrier

  // Benchmark.
  for (auto _ : state) {
    // Launch `num_threads - 1` additional threads, but pause the timer to avoid
    // measuring the overhead of creating the additional threads.
    state.PauseTiming();
    for (int thread_index = 1; thread_index < num_threads; ++thread_index) {
      threads.emplace_back([&] {
        HitBarrier(barrier, policy, num_threads, barrier_iterations);
      });
    }
    state.ResumeTiming();
    // Main thread also contends. Measure the time it takes to the main thread
    // to complete a number of `barrier_iterations` while contending with the
    // other threads at each stage.
    HitBarrier(barrier, policy, num_threads, barrier_iterations);
    // Prepare for next benchmark iteration, but pause the timer to avoid
    // measuring the overhead of joining the additional threads.
    state.PauseTiming();
    for (auto& thread : threads) thread.join();
    threads.clear();
    state.ResumeTiming();
  }

  // Teardown.
  state.SetItemsProcessed(state.iterations() * barrier_iterations);
  state.SetLabel(bm_label(type, policy, barrier_iterations));
  delete barrier;
}
BENCHMARK(BM_BarrierIters)
    ->Apply(BarrierItersArgs)
    ->ArgNames({"barrier_type", "waiting_policy", "threads"})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
