// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/include/modcncy/barrier.h"

#include "modcncy/src/primitives/barriers/central_sense_counter_barrier.h"
#include "modcncy/src/primitives/barriers/central_step_counter_barrier.h"

namespace modcncy {

// =============================================================================
// Factory method. Creates a new `Barrier` object based on its type.
Barrier* Barrier::Create(BarrierType type) {
  switch (type) {
    case BarrierType::kCentralSenseCounterBarrier:
      return new primitives::CentralSenseCounterBarrier();
    case BarrierType::kCentralStepCounterBarrier:
      return new primitives::CentralStepCounterBarrier();
  }
  return nullptr;
}

}  // namespace modcncy
