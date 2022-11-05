// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// The `CentralStepCounterBarrier` is a simple barrier implementation where a
// central counter is shared among multiple execution threads and notifications
// to other threads is done by keeping track of the number of times the barrier
// has been used so far (current step). Its behavior is summarized as follows:
//
//   1. When a thread arrives at the barrier, it increases the shared counter
//      and starts spinning on the number of barrier synchronizations completed
//      so far.
//
//   2. When the last thread arrives at the barrier, it resets the shared
//      counter and moves all current spinning threads out of the barrier by
//      increasing the current step.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_STEP_COUNTER_BARRIER_H_
#define MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_STEP_COUNTER_BARRIER_H_

#include <atomic>

#include "modcncy/include/modcncy/barrier.h"
#include "modcncy/include/modcncy/global_expressions.h"

namespace modcncy {
namespace primitives {

class CentralStepCounterBarrier : public Barrier {
 public:
  // A thread must wait here until all threads reach this point.
  void Wait(int num_threads, std::function<void()> policy) override;

 private:
  // Number of threads spinning at the barrier.
  std::atomic<int> spinning_threads_{0};

  // Padding to prevent false sharing.
  char padding_[kCacheLineSize - sizeof(std::atomic<int>)];

  // Number of barrier synchronizations completed so far.
  // The barrier is reusable since unsigned data type wraps around the overflow.
  std::atomic<unsigned> step_{0};
};  // class CentralStepCounterBarrier

}  // namespace primitives
}  // namespace modcncy

#endif  // MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_STEP_COUNTER_BARRIER_H_
