// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "examples/bitonicsort/src/bitonicsort.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

// =============================================================================
TEST(BitonicsortTest, SimpleIntegersSorting) {
  std::vector<int> unsorted = {5, 7, 1, 4, 8, 2, 3, 6};
  bitonicsort::sort(unsorted.begin(), unsorted.end(), /*segment_size=*/2);
  EXPECT_THAT(unsorted, testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));
}
