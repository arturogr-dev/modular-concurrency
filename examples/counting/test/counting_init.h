// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Init test and benchmark configurations for the counting example.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_COUNTING_TEST_COUNTING_INIT_H_
#define EXAMPLES_COUNTING_TEST_COUNTING_INIT_H_

#include <modcncy/flags.h>

namespace counting {

// Declare command line flags to be used.
MODCNCY_DECLARE_int32(increments_per_thread);

// =============================================================================
// Parses the declared command line flags.
void ParseCommandLineFlags(int* argc, char** argv) {
  for (int i = 1; i < *argc; ++i) {
    if (modcncy::ParseInt32Flag(argv[i], "increments_per_thread",
                                &FLAGS_increments_per_thread)) {
      for (int j = i; j != *argc - 1; ++j) argv[j] = argv[j + 1];
      --(*argc);
      --i;
    }
  }
}

}  // namespace counting

#endif  // EXAMPLES_COUNTING_TEST_COUNTING_INIT_H_
