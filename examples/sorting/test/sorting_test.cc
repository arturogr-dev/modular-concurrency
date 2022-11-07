// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <vector>

#include "examples/sorting/include/algorithm.h"

namespace sorting {
namespace {

class SortingCorrectnessTest : public testing::TestWithParam<SortType> {};

INSTANTIATE_TEST_SUITE_P(
    AllSortingTypes, SortingCorrectnessTest,
    testing::Values(SortType::kSequentialStdSort,
                    SortType::kSequentialOriginalBitonicsort,
                    SortType::kSequentialSegmentedBitonicsort,
                    SortType::kParallelOmpBasedBitonicsort,
                    SortType::kParallelPthreadsBitonicsort,
                    SortType::kParallelNonBlockingBitonicsort,
                    SortType::kParallelGnuMultiwayMergesort,
                    SortType::kParallelGnuQuicksort,
                    SortType::kParallelGnuBalancedQuicksort));

// =============================================================================
TEST_P(SortingCorrectnessTest, Sort32BitInts) {
  constexpr size_t size = 1024;
  std::vector<int32_t> sorted(size);
  std::vector<int32_t> unsorted(size);
  for (size_t i = 0; i < size; ++i) {
    unsorted[i] = size - i - 1;
    sorted[i] = i;
  }
  sorting::sort(unsorted.begin(), unsorted.end(), /*sort_type=*/GetParam());
  EXPECT_EQ(unsorted, sorted);
}

}  // namespace
}  // namespace sorting
