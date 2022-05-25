// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "primitives/barriers/central_sense_counter_barrier.h"

namespace modcncy {
namespace primitives {

// =============================================================================
void CentralSenseCounterBarrier::Wait(int num_threads, WaitingPolicy policy) {
  const unsigned my_sense = sense_.load(std::memory_order_relaxed);
  if (spinning_threads_.fetch_add(1, std::memory_order_acq_rel) <
      num_threads - 1) {
    // Wait until last thread arrives.
    while (sense_.load(std::memory_order_acquire) == my_sense)
      WaitingWithPolicy(policy);
  } else {
    // Last thread enters the barrier.
    // Reset number of spinning threads and increase the step.
    spinning_threads_.store(0, std::memory_order_relaxed);
    sense_.store(~my_sense, std::memory_order_release);
  }
}

}  // namespace primitives
}  // namespace modcncy
