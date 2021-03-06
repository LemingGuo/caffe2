/**
 * Copyright (c) 2016-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>  // NOLINT

#include "caffe2/utils/simple_queue.h"
#include <gtest/gtest.h>

namespace caffe2 {

static std::unique_ptr<SimpleQueue<int> > gQueue;

static void ConsumerFunction(int thread_idx) {
  int value;
  while (true) {
    if (!gQueue->Pop(&value)) return;
    VLOG(1) << "Emitting " << value << " from thread " << thread_idx;
  }
}

static void ProducerFunction(int thread_idx, int start, int count) {
  for (int i = 0; i < count; ++i) {
    VLOG(1) << "Pushing " << i + start << " from thread " << thread_idx;
    gQueue->Push(i + start);
  }
}


TEST(SimpleQueueTest, SingleProducerSingleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread consumer(ConsumerFunction, 0);
  for (int i = 0; i < 10; ++i) {
    gQueue->Push(i);
  }
  gQueue->NoMoreJobs();
  consumer.join();
}

TEST(SimpleQueueTest, SingleProducerDoubleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread consumer0(ConsumerFunction, 0);
  std::thread consumer1(ConsumerFunction, 1);
  for (int i = 0; i < 10; ++i) {
    gQueue->Push(i);
  }
  gQueue->NoMoreJobs();
  consumer0.join();
  consumer1.join();
}


TEST(SimpleQueueTest, DoubleProducerDoubleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread producer0(ProducerFunction, 0, 0, 10);
  std::thread producer1(ProducerFunction, 0, 10, 10);
  std::thread consumer0(ConsumerFunction, 2);
  std::thread consumer1(ConsumerFunction, 3);
  producer0.join();
  producer1.join();
  gQueue->NoMoreJobs();
  consumer0.join();
  consumer1.join();
}

TEST(SimpleQueueDeathTest, CannotAddAfterQueueFinished) {
  gQueue.reset(new SimpleQueue<int>());
  gQueue->Push(0);
  gQueue->NoMoreJobs();
  ASSERT_THROW(gQueue->Push(0), EnforceNotMet);
}


}  // namespace caffe2


