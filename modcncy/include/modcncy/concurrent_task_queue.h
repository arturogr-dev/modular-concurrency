// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// A concurrent task queue is a thread-safe FIFO container of tasks.
//
// A factory is in charge of instantiating any of the different supported
// concurrent task queue implementations during runtime.
//
// The template to be followed by any concurrent task queue implementation:
//
//   + `Push()` inserts a task into the queue.
//
//   + `Pop()` removes a task from the queue.
//
// TODO(arturogr-dev): Add usage example.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_INCLUDE_MODCNCY_CONCURRENT_TASK_QUEUE_H_
#define MODCNCY_INCLUDE_MODCNCY_CONCURRENT_TASK_QUEUE_H_

#include <functional>

namespace modcncy {

// Supported concurrent task queues.
enum class ConcurrentTaskQueueType {
  kBlockingTaskQueue = 0,  // Concurrent blocking queue of tasks.
};

// Concurrent task queue base interface.
class ConcurrentTaskQueue {
 public:
  // Factory method. Creates a new `Barrier` object.
  static ConcurrentTaskQueue* Create(ConcurrentTaskQueueType type);

  virtual ~ConcurrentTaskQueue() {}

  // Inserts a task into the queue.
  virtual void Push(std::function<void()> task) = 0;

  // Removes a task from the queue.
  virtual std::function<void()> Pop() = 0;
};  // class ConcurrentTaskQueue

}  // namespace modcncy

#endif  // MODCNCY_INCLUDE_MODCNCY_CONCURRENT_TASK_QUEUE_H_
