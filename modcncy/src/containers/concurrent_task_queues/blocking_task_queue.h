// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// The `BlockingTaskQueue` is a simple thread-safe blocking concurrent FIFO
// queue of tasks.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_SRC_CONTAINERS_CONCURRENT_TASK_QUEUES_BLOCKING_TASK_QUEUE_H_
#define MODCNCY_SRC_CONTAINERS_CONCURRENT_TASK_QUEUES_BLOCKING_TASK_QUEUE_H_

#include <deque>
#include <mutex>  // NOLINT(build/c++11)

#include "modcncy/include/modcncy/concurrent_task_queue.h"

namespace modcncy {
namespace containers {

class BlockingTaskQueue : public ConcurrentTaskQueue {
 public:
  // Inserts a task into the queue.
  void Push(std::function<void()> task) override;

  // Removes a task from the queue.
  std::function<void()> Pop() override;

 private:
  // Protects the concurrent reads/writes from/to the queue.
  std::mutex mutex_;

  // Using `std::deque` for pointer consistency and FIFO order.
  std::deque<std::function<void()>> queue_;
};  // class BlockingTaskQueue

}  // namespace containers
}  // namespace modcncy

#endif  // MODCNCY_SRC_CONTAINERS_CONCURRENT_TASK_QUEUES_BLOCKING_TASK_QUEUE_H_
