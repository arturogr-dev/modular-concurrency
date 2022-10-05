// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/src/primitives/barriers/barrier_factory.h"

namespace modcncy {

// =============================================================================
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
