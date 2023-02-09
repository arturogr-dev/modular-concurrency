// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// The `AtomicCounter` is a simple implementation where all execution threads
// increment the same shared variable.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_COUNTING_INCLUDE_ATOMIC_COUNTER_H_
#define EXAMPLES_COUNTING_INCLUDE_ATOMIC_COUNTER_H_

#include <atomic>

#include "examples/counting/include/algorithm.h"

namespace counting {

class AtomicCounter : public Counter {
 public:
  void Increment() override { count_.fetch_add(1, std::memory_order_relaxed); }
  void Reset() override { count_.store(0, std::memory_order_relaxed); }
  size_t Count() override { return count_.load(std::memory_order_relaxed); }

 private:
  std::atomic<size_t> count_{0};
};  // class AtomicCounter

}  // namespace counting

#endif  // EXAMPLES_COUNTING_INCLUDE_ATOMIC_COUNTER_H_
