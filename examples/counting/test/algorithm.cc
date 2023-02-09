// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "examples/counting/include/algorithm.h"

#include "examples/counting/include/atomic_counter.h"

namespace counting {

// =============================================================================
// Factory method. Creates a new `Counter` object based on its type.
Counter* Counter::Create(CounterType type) {
  switch (type) {
    case CounterType::kAtomicCounter:
      return new AtomicCounter();
  }
  return nullptr;
}

}  // namespace counting
