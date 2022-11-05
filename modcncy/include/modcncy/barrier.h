// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// A Barrier is a synchronization primitive that guarantees that no thread can
// continue execution of a program at a given point until all other threads
// reach that same point.
//
// A factory is in charge of instantiating any of the different supported
// barrier implementations during runtime.
//
// The template to be followed by any barrier implementation:
//
//   + `Wait()` must guarantee to stop the execution of a thread until all other
//     threads reach this same point.
//
// TODO(arturogr-dev): Add usage example.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_INCLUDE_MODCNCY_BARRIER_H_
#define MODCNCY_INCLUDE_MODCNCY_BARRIER_H_

#include <functional>

#include "modcncy/wait_policy.h"

namespace modcncy {

// Supported barriers.
enum class BarrierType {
  kCentralSenseCounterBarrier = 0,  // Central Sense and Central Counter Barrier
  kCentralStepCounterBarrier = 1,   // Central Step and Central Counter Barrier
};

// Barrier base interface.
class Barrier {
 public:
  // Factory method. Creates a new `Barrier` object.
  static Barrier* Create(BarrierType type);

  virtual ~Barrier() {}

  // Blocks current thread until the last of `num_threads` reaches this point.
  // All threads at the barrier wait with the applied `policy`.
  virtual void Wait(int num_threads,
                    std::function<void()> policy = &cpu_yield) = 0;
};  // class Barrier

}  // namespace modcncy

#endif  // MODCNCY_INCLUDE_MODCNCY_BARRIER_H_
