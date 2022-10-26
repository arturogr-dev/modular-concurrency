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

namespace merge {

// =============================================================================
// Copies the data from buffer to two segments of the same size. The buffer is
// two times the size of each segment. The first half of the buffer is copied to
// `segment1` and the second half of the buffer is copied to `segment2`.
template <typename T>
static inline void Scatter(T* buffer, T* segment1, T* segment2, int size) {
  const int bytes = size * sizeof(int);
  std::memcpy(/*dst=*/segment1, /*src=*/buffer, /*cnt=*/bytes);
  std::memcpy(/*dst=*/segment2, /*src=*/buffer + size, /*cnt=*/bytes);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-decreasing order.
template <typename T>
static void MergeUpFromUpUp(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = 0, j = 0, k = 0;
  while (i < size && j < size) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j++];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j < size) buffer[k++] = segment2[j++];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-increasing order.
template <typename T>
static void MergeUpFromUpDn(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = 0, j = size - 1, k = 0;
  while (i < size && j >= 0) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j--];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j >= 0) buffer[k++] = segment2[j--];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-decreasing order.
template <typename T>
static void MergeUpFromDnUp(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = size - 1, j = 0, k = 0;
  while (i >= 0 && j < size) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j++];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j < size) buffer[k++] = segment2[j++];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-increasing order.
template <typename T>
static void MergeUpFromDnDn(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = size - 1, j = size - 1, k = 0;
  while (i >= 0 && j >= 0) {
    if (segment1[i] < segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j--];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j >= 0) buffer[k++] = segment2[j--];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-decreasing order.
template <typename T>
static void MergeDnFromUpUp(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = size - 1, j = size - 1, k = 0;
  while (i >= 0 && j >= 0) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j--];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j >= 0) buffer[k++] = segment2[j--];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-decreasing order.
// `segment2` is in non-increasing order.
template <typename T>
static void MergeDnFromUpDn(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = size - 1, j = 0, k = 0;
  while (i >= 0 && j < size) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i--];
    else
      buffer[k++] = segment2[j++];
  }
  while (i >= 0) buffer[k++] = segment1[i--];
  while (j < size) buffer[k++] = segment2[j++];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-decreasing order.
template <typename T>
static void MergeDnFromDnUp(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = 0, j = size - 1, k = 0;
  while (i < size && j >= 0) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j--];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j >= 0) buffer[k++] = segment2[j--];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
// `segment2` is in non-increasing order.
// `segment2` is in non-increasing order.
template <typename T>
static void MergeDnFromDnDn(T* segment1, T* segment2, T* buffer, int size) {
  // Merge into buffer.
  int i = 0, j = 0, k = 0;
  while (i < size && j < size) {
    if (segment1[i] > segment2[j])
      buffer[k++] = segment1[i++];
    else
      buffer[k++] = segment2[j++];
  }
  while (i < size) buffer[k++] = segment1[i++];
  while (j < size) buffer[k++] = segment2[j++];
  // Scatter data back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
// Merges two segments of the same size in non-decreasing order without knowing
// the order of the input segments.
template <typename T>
static void MergeUp(T* segment1, T* segment2, T* buffer, int size) {
  const int l = 0, r = size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    MergeUpFromUpUp(segment1, segment2, buffer, size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    MergeUpFromUpDn(segment1, segment2, buffer, size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    MergeUpFromDnUp(segment1, segment2, buffer, size);
  else
    MergeUpFromDnDn(segment1, segment2, buffer, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order without knowing
// the order of the input segments.
template <typename T>
static void MergeDn(T* segment1, T* segment2, T* buffer, int size) {
  const int l = 0, r = size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    MergeDnFromUpUp(segment1, segment2, buffer, size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    MergeDnFromUpDn(segment1, segment2, buffer, size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    MergeDnFromDnUp(segment1, segment2, buffer, size);
  else
    MergeDnFromDnDn(segment1, segment2, buffer, size);
}

}  // namespace merge

#endif  // EXAMPLES_SORTING_INCLUDE_MERGE_H_
