// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Helper functions to merge segments of data of the same size. The merging is
// performed in place. This is, the result is stored in the original data
// segments. However, for practical reasons, a helper buffer is used in order to
// perform the merge in linear-time. The source file contains more details on
// each function. It includes all possible merging combinations, for example
// merging in increasing order when one input segment is sorted in increasing
// order and the other one is sorted in decreasing order (and so on and so
// forth).
//
// -----------------------------------------------------------------------------

#ifndef EXAMPLES_BITONICSORT_SRC_MERGE_H_
#define EXAMPLES_BITONICSORT_SRC_MERGE_H_

#include <cstring>

////////////////////////////////////////////////////////////////////////////////
///////////////////////////  D E F I N I T I O N S  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace bitonicsort {

namespace internal {

// Scatter data back into original arrays.
template <typename T>
static void Scatter(T* buffer, T* segment1, T* segment2, int size);

// All possible configurations to merge pair of segments of the same size,
// depending on the sorting direction of the given segments.
template <typename T>
static void MergeUpFromUpUp(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeUpFromUpDn(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeUpFromDnUp(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeUpFromDnDn(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeDnFromUpUp(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeDnFromUpDn(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeDnFromDnUp(T* segment1, T* segment2, T* buffer, int size);
template <typename T>
static void MergeDnFromDnDn(T* segment1, T* segment2, T* buffer, int size);

}  // namespace internal

// =============================================================================
// Merges two segments of the same size in non-decreasing order.
template <typename T>
void MergeUp(T* segment1, T* segment2, T* buffer, int size) {
  const int l = 0, r = size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    internal::MergeUpFromUpUp(segment1, segment2, buffer, size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    internal::MergeUpFromUpDn(segment1, segment2, buffer, size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    internal::MergeUpFromDnUp(segment1, segment2, buffer, size);
  else
    internal::MergeUpFromDnDn(segment1, segment2, buffer, size);
}

// =============================================================================
// Merges two segments of the same size in non-increasing order.
template <typename T>
void MergeDn(T* segment1, T* segment2, T* buffer, int size) {
  const int l = 0, r = size - 1;
  if (segment1[l] < segment1[r] && segment2[l] < segment2[r])
    internal::MergeDnFromUpUp(segment1, segment2, buffer, size);
  else if (segment1[l] < segment1[r] && segment2[l] > segment2[r])
    internal::MergeDnFromUpDn(segment1, segment2, buffer, size);
  else if (segment1[l] > segment1[r] && segment2[l] < segment2[r])
    internal::MergeDnFromDnUp(segment1, segment2, buffer, size);
  else
    internal::MergeDnFromDnDn(segment1, segment2, buffer, size);
}

}  // namespace bitonicsort

////////////////////////////////////////////////////////////////////////////////
///////////////////////  I M P L E M E N T A T I O N S  ////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace bitonicsort {
namespace internal {

// =============================================================================
template <typename T>
void Scatter(T* buffer, T* segment1, T* segment2, int size) {
  const int bytes = size * sizeof(int);
  std::memcpy(/*dst=*/segment1, /*src=*/buffer, /*cnt=*/bytes);
  std::memcpy(/*dst=*/segment2, /*src=*/buffer + size, /*cnt=*/bytes);
}

// =============================================================================
template <typename T>
void MergeUpFromUpUp(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeUpFromUpDn(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeUpFromDnUp(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeUpFromDnDn(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeDnFromUpUp(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeDnFromUpDn(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeDnFromDnUp(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

// =============================================================================
template <typename T>
void MergeDnFromDnDn(T* segment1, T* segment2, T* buffer, int size) {
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

  // Scatter back into original arrays.
  Scatter(buffer, segment1, segment2, size);
}

}  // namespace internal
}  // namespace bitonicsort

#endif  // EXAMPLES_BITONICSORT_SRC_MERGE_H_
