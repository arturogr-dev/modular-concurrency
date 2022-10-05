// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// In shared memory architectures, when an execution thread waits to synchronize
// with another operation, it is usually implemented as a busy-wait technique.
// These waiting techniques are known as spin-wait loops.
//
// There are different actions that a thread can take in this situation while
// waiting for a condition to be true:
//
//   + Active Waiting: The thread spins without giving up the processor and it
//     is "actively" consuming CPU cycles.
//
//   + Passive Waiting: The thread yields the processor and it "passively" waits
//     to be put back on the CPU again.
//
//   + Paused Waiting: The thread hints the processor to "pause" and it can help
//     optimize CPU performance and power consumption.
//
// Note:
//
//   For more information on the "Paused Waiting" technique, search for the
//   `_mm_pause` intrinsic at:
//
//   - https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
//
//   If the instrinsic `void _mm_pause(void)` is supported by the CPU, it is
//   translated to a `PAUSE` instruction. However, if it is not supported, it is
//   safely decoded as a `NOP`. Some references:
//
//   - https://c9x.me/x86/html/file_module_x86_id_232.html
//   - https://www.felixcloutier.com/x86/pause
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_INCLUDE_MODCNCY_WAIT_POLICY_H_
#define MODCNCY_INCLUDE_MODCNCY_WAIT_POLICY_H_

#include <emmintrin.h>

#include <thread>  // NOLINT(build/c++11)

namespace modcncy {

// =============================================================================
// Support for active waiting. Spins consuming CPU cycles.
inline void cpu_no_op() {}

// =============================================================================
// Support for passive waiting. Hints to yield the CPU to other threads.
inline void cpu_yield() { std::this_thread::yield(); }

// =============================================================================
// Support for paused waiting. Tries to optimize the spin-wait loop.
inline void cpu_pause() { _mm_pause(); }

}  // namespace modcncy

#endif  // MODCNCY_INCLUDE_MODCNCY_WAIT_POLICY_H_
