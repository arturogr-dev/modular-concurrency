// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/include/modcncy/concurrent_task_queue.h"

#include "modcncy/src/containers/concurrent_task_queues/blocking_task_queue.h"

namespace modcncy {

// =============================================================================
// Factory method. Creates a new `ConcurrentTaskQueue` object based on its type.
ConcurrentTaskQueue* ConcurrentTaskQueue::Create(ConcurrentTaskQueueType type) {
  switch (type) {
    case ConcurrentTaskQueueType::kBlockingTaskQueue:
      return new containers::BlockingTaskQueue();
  }
  return nullptr;
}

}  // namespace modcncy
