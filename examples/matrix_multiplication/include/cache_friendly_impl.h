// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Cache-friendly matrix multiplication.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_CACHE_FRIENDLY_IMPL_H_
#define EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_CACHE_FRIENDLY_IMPL_H_

#include <omp.h>

#include <vector>

namespace matrix_multiplication {
namespace cache_friendly_impl {

// =============================================================================
// Matrix-A x Matrix-B sequential cache-friendly implementation.
template <typename T>
std::vector<std::vector<T>> sequential(const std::vector<std::vector<T>>& A,
                                       const std::vector<std::vector<T>>& B) {
  std::vector<std::vector<T>> result(A.size(), std::vector<T>(B[0].size()));
  for (size_t k = 0; k < A[0].size(); ++k) {
    for (size_t i = 0; i < A.size(); ++i) {
      for (size_t j = 0; j < B[0].size(); ++j) {
        result[i][j] += A[i][k] * B[k][j];
      }
    }
  }
  return result;
}

// =============================================================================
// Matrix-A x Matrix-B parallel cache-friendly implementation.
template <typename T>
std::vector<std::vector<T>> parallel(const std::vector<std::vector<T>>& A,
                                     const std::vector<std::vector<T>>& B,
                                     size_t num_threads) {
  omp_set_dynamic(0);
  omp_set_num_threads(num_threads);
  std::vector<std::vector<T>> result(A.size(), std::vector<T>(B[0].size()));
#pragma omp parallel shared(A, B, result)
  {
    for (size_t k = 0; k < A[0].size(); ++k) {
#pragma omp for nowait
      for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < B[0].size(); ++j) {
          result[i][j] += A[i][k] * B[k][j];
        }
      }
    }
  }  // pragma omp parallel
  return result;
}

}  // namespace cache_friendly_impl
}  // namespace matrix_multiplication

#endif  // EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_CACHE_FRIENDLY_IMPL_H_
