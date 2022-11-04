// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Init test and benchmark configurations for the sorting examples.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_TEST_SORTING_INIT_H_
#define EXAMPLES_SORTING_TEST_SORTING_INIT_H_

#include <modcncy/flags.h>

namespace sorting {

// Declare command line flags to be used.
MODCNCY_DECLARE_int32(input_shift);
MODCNCY_DECLARE_int32(segment_size);
MODCNCY_DECLARE_int32(num_threads);

// =============================================================================
// Parses the declared command line flags.
void ParseCommandLineFlags(int* argc, char** argv) {
  for (int i = 1; i < *argc; ++i) {
    if (modcncy::ParseInt32Flag(argv[i], "input_shift", &FLAGS_input_shift) ||
        modcncy::ParseInt32Flag(argv[i], "segment_size", &FLAGS_segment_size) ||
        modcncy::ParseInt32Flag(argv[i], "num_threads", &FLAGS_num_threads)) {
      for (int j = i; j != *argc - 1; ++j) argv[j] = argv[j + 1];
      --(*argc);
      --i;
    }
  }
}

}  // namespace sorting

#endif  // EXAMPLES_SORTING_TEST_SORTING_INIT_H_
