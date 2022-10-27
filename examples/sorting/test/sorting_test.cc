// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "examples/sorting/include/sorting.h"

#include <gtest/gtest.h>

#include <vector>

class SortingTest : public testing::TestWithParam<sorting::Type> {};

INSTANTIATE_TEST_SUITE_P(AllSortingTypes, SortingTest,
                         testing::Values(sorting::Type::kStdSort,
                                         sorting::Type::kOriginalBitonicsort,
                                         sorting::Type::kSegmentedBitonicsort,
                                         sorting::Type::kOmpBasedBitonicsort,
                                         sorting::Type::kNonBlockingBitonicsort,
                                         sorting::Type::kGnuMultiwayMergesort));

// =============================================================================
TEST_P(SortingTest, Sort32BitInts) {
  constexpr size_t size = 1024;
  std::vector<int32_t> sorted;
  std::vector<int32_t> unsorted;
  sorted.reserve(size);
  unsorted.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    unsorted.push_back(size - i - 1);
    sorted.push_back(i);
  }
  sorting::sort(unsorted.begin(), unsorted.end(), /*type=*/GetParam());
  EXPECT_EQ(unsorted, sorted);
}
