// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/src/containers/concurrent_task_queues/blocking_task_queue.h"

#include <utility>

namespace modcncy {
namespace containers {

// =============================================================================
void BlockingTaskQueue::Push(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push_back(task);
}

// =============================================================================
std::function<void()> BlockingTaskQueue::Pop() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!queue_.empty()) {
    std::function<void()> task = std::move(queue_.front());
    queue_.pop_front();
    return task;
  }
  return nullptr;
}

}  // namespace containers
}  // namespace modcncy
