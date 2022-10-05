// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// The `CentralSenseCounterBarrier` is a simple barrier implementation where a
// central counter is shared among multiple execution threads and notifications
// to other threads is done via a global sense flag. Its behavior is summarized
// as follows:
//
//   1. When a thread arrives at the barrier, it increases the shared counter
//      and starts spinning on the global sense flag.
//
//   2. When the last thread arrives at the barrier, it resets the shared
//      counter and moves all current spinning threads out of the barrier by
//      flipping the global sense flag.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_SENSE_COUNTER_BARRIER_H_
#define MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_SENSE_COUNTER_BARRIER_H_

#include <atomic>

#include "modcncy/include/modcncy/barrier.h"
#include "modcncy/include/modcncy/global_expressions.h"

namespace modcncy {
namespace primitives {

class CentralSenseCounterBarrier : public modcncy::Barrier {
 public:
  // A thread must wait here until all threads reach this point.
  void Wait(int num_threads) override;

 private:
  // Number of threads spinning at the barrier.
  std::atomic<int> spinning_threads_{0};

  // Padding to prevent false sharing.
  char padding_[modcncy::kCacheLineSize - sizeof(std::atomic<int>)];

  // Global sense flag.
  // The barrier is reusable since it flips between states.
  std::atomic<unsigned> sense_{0};
};  // class CentralSenseCounterBarrier

}  // namespace primitives
}  // namespace modcncy

#endif  // MODCNCY_SRC_PRIMITIVES_BARRIERS_CENTRAL_SENSE_COUNTER_BARRIER_H_
