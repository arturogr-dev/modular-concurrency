// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/src/primitives/barriers/barrier_factory.h"

namespace modcncy {

// =============================================================================
// Factory.
modcncy::Barrier* Barrier::Create(modcncy::BarrierType type) {
  switch (type) {
    case modcncy::BarrierType::kCentralSenseCounterBarrier:
      return new modcncy::primitives::CentralSenseCounterBarrier();
    case modcncy::BarrierType::kCentralStepCounterBarrier:
      return new modcncy::primitives::CentralStepCounterBarrier();
  }
  return nullptr;
}

}  // namespace modcncy
