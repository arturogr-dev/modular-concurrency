// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Helper functions to merge segments of data of the same size. The merging is
// performed in place. This is, the result is stored in the original data
// segments. However, for practical reasons, a helper buffer is used in order to
// perform the merge in linear time. The source file contains more details on
// each function. It includes all possible merging combinations, for example
// merging in increasing order when one input segment is sorted in increasing
// order and the other one is sorted in decreasing order (and so on and so
// forth).
//
// TODO(arturogr-dev): Add usage example.
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_SORTING_INCLUDE_MERGE_H_
#define EXAMPLES_SORTING_INCLUDE_MERGE_H_

#include <cstring>

namespace sorting {
namespace merge {

// =============================================================================
// Copies the data from buffer to two segments of the same size. The buffer is
// two times the size of each segment. The first half of the buffer is copied to
// `segment1` and the second half of the buffer is copied to `segment2`.
template <typename T>
static void Scatter(T* buffer, T* segment1, T* segment2, size_t segment_size) {
  const size_t bytes = segment_size * sizeof(T);
  std::memcpy(/*dst=*/segment1, /*src=*/buffer, /*cnt=*/bytes);
  std::memcpy(/*dst=*/segment2, /*src=*/buffer + segment_size, /*cnt=*/bytes);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-decreasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void UpFromUpUp(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = 0, j = 0, k = 0;
  while (i < size && j < size) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j++];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j < size) buffer[k++] = segment2[j++];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void UpFromUpDn(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = 0, j = size - 1, k = 0;
  while (i < size && j >= 0) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j--];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j >= 0) buffer[k++] = segment2[j--];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-decreasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void UpFromDnUp(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = size - 1, j = 0, k = 0;
  while (i >= 0 && j < size) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j++];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j < size) buffer[k++] = segment2[j++];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void UpFromDnDn(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = size - 1, j = size - 1, k = 0;
  while (i >= 0 && j >= 0) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j--];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j >= 0) buffer[k++] = segment2[j--];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-decreasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void DnFromUpUp(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = size - 1, j = size - 1, k = 0;
  while (i >= 0 && j >= 0) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j--];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j >= 0) buffer[k++] = segment2[j--];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void DnFromUpDn(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = size - 1, j = 0, k = 0;
  while (i >= 0 && j < size) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j++];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j < size) buffer[k++] = segment2[j++];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-decreasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void DnFromDnUp(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = 0, j = size - 1, k = 0;
  while (i < size && j >= 0) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j--];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j >= 0) buffer[k++] = segment2[j--];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void DnFromDnDn(T* segment1, T* segment2, T* buffer,
                       size_t segment_size) {
  int64_t size = static_cast<int64_t>(segment_size);
  int64_t i = 0, j = 0, k = 0;
  while (i < size && j < size) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j++];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j < size) buffer[k++] = segment2[j++];
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order without knowing
// whether they are sorted in non-decreasing or non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void Up(T* segment1, T* segment2, T* buffer, size_t segment_size) {
  const size_t l = 0, r = segment_size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    UpFromUpUp(segment1, segment2, buffer, segment_size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    UpFromUpDn(segment1, segment2, buffer, segment_size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    UpFromDnUp(segment1, segment2, buffer, segment_size);
  else
    UpFromDnDn(segment1, segment2, buffer, segment_size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order without knowing
// whether they are sorted in non-decreasing or non-increasing order.
// The buffer is two times the size of each segment.
template <typename T>
static void Dn(T* segment1, T* segment2, T* buffer, size_t segment_size) {
  const size_t l = 0, r = segment_size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    DnFromUpUp(segment1, segment2, buffer, segment_size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    DnFromUpDn(segment1, segment2, buffer, segment_size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    DnFromDnUp(segment1, segment2, buffer, segment_size);
  else
    DnFromDnDn(segment1, segment2, buffer, segment_size);
}

}  // namespace merge
}  // namespace sorting

#endif  // EXAMPLES_SORTING_INCLUDE_MERGE_H_
