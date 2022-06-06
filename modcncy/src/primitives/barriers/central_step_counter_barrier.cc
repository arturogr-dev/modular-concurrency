// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/src/primitives/barriers/central_step_counter_barrier.h"

namespace modcncy {
namespace primitives {

// =============================================================================
void CentralStepCounterBarrier::Wait(int num_threads, WaitPolicy policy) {
  const unsigned current_step = step_.load(std::memory_order_relaxed);
  if (spinning_threads_.fetch_add(1, std::memory_order_acq_rel) <
      num_threads - 1) {
    // Wait until last thread arrives.
    while (step_.load(std::memory_order_acquire) == current_step)
      WaitWithPolicy(policy);
  } else {
    // Last thread enters the barrier.
    // Reset number of spinning threads and increase the step.
    spinning_threads_.store(0, std::memory_order_relaxed);
    step_.fetch_add(1, std::memory_order_release);
  }
}

}  // namespace primitives
}  // namespace modcncy
