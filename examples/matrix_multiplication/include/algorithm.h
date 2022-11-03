// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Matrix multiplication implementations.
//
// TODO(arturogr-dev): Add example usage.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_ALGORITHM_H_
#define EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_ALGORITHM_H_

#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "examples/matrix_multiplication/include/cache_friendly_impl.h"
#include "examples/matrix_multiplication/include/naive_impl.h"

namespace matrix_multiplication {

// Supported execution policies.
enum class MultiplyType {
  kSequentialNaive = 0,          // Well-known O(n^3) implementation.
  kSequentialCacheFriendly = 1,  // Same as naive but exploits cache hierarchy.
  kParallelNaive = 2,            // Multithreaded naive implementation.
  kParallelCacheFriendly = 3,    // Multithreaded cache-friendly implementation.
};

// =============================================================================
// Main function to execute the different matrix multiplication algorithms.
template <typename T>
std::vector<std::vector<T>> multiply(
    const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& B,
    MultiplyType multiply_type = MultiplyType::kSequentialNaive,
    size_t num_threads = std::thread::hardware_concurrency()) {
  switch (multiply_type) {
    case MultiplyType::kSequentialNaive:
      return naive_impl::sequential(A, B);
    case MultiplyType::kSequentialCacheFriendly:
      return cache_friendly_impl::sequential(A, B);
    case MultiplyType::kParallelNaive:
      return naive_impl::parallel(A, B, num_threads);
    case MultiplyType::kParallelCacheFriendly:
      return cache_friendly_impl::parallel(A, B, num_threads);
  }
  return {};
}

}  // namespace matrix_multiplication

#endif  // EXAMPLES_MATRIX_MULTIPLICATION_INCLUDE_ALGORITHM_H_
