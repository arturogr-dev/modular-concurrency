// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Atomic counting.
//
// TODO(arturogr-dev): Add example.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_COUNTING_INCLUDE_ALGORITHM_H_
#define EXAMPLES_COUNTING_INCLUDE_ALGORITHM_H_

#include <cstddef>

namespace counting {

// Supported counters.
enum class CounterType {
  kAtomicCounter = 0,  // One shared atomic variable across threads.
};

// Counter base interface.
class Counter {
 public:
  // Factory method. Creates a new `Counter` object.
  static Counter* Create(CounterType type);

  virtual ~Counter() {}

  // Increments the counter.
  virtual void Increment() = 0;

  // Resets the counter.
  virtual void Reset() = 0;

  // Returns the current count.
  virtual size_t Count() = 0;
};  // class Counter

}  // namespace counting

#endif  // EXAMPLES_COUNTING_INCLUDE_ALGORITHM_H_
