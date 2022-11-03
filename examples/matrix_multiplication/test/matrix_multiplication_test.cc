// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "examples/matrix_multiplication/include/algorithm.h"

namespace matrix_multiplication {
namespace {

class MatrixMultiplicationTest : public testing::TestWithParam<MultiplyType> {};

INSTANTIATE_TEST_SUITE_P(AllMultiplyTypes, MatrixMultiplicationTest,
                         testing::Values(MultiplyType::kSequentialNaive,
                                         MultiplyType::kSequentialCacheFriendly,
                                         MultiplyType::kParallelNaive,
                                         MultiplyType::kParallelCacheFriendly));

// =============================================================================
TEST_P(MatrixMultiplicationTest, Multiply32BitIntsMatrices) {
  std::vector<std::vector<int32_t>> A = {{1, 2, 3}, {4, 5, 6}};
  std::vector<std::vector<int32_t>> B = {{7, 8}, {9, 10}, {11, 12}};
  std::vector<std::vector<int32_t>> C = {{58, 64}, {139, 154}};
  EXPECT_EQ(multiply(A, B, /*multiply_type=*/GetParam(), /*num_threads=*/2), C);
}

}  // namespace
}  // namespace matrix_multiplication
