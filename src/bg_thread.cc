// Copyright (c) 2015-present, Qihoo, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
#include "bg_thread.h"

namespace pink {
void BGThread::Schedule(void (*function)(void*), void* arg) {
  pthread_mutex_lock(&mu_);
  while (queue_.size() >= full_ && !should_exit_) {
    pthread_cond_wait(&wsignal_, &mu_);
  }
  if (queue_.size() < full_) {
    queue_.push_back(BGItem(function, arg));
    pthread_cond_signal(&rsignal_);
  }
  pthread_mutex_unlock(&mu_);
}

void *BGThread::ThreadMain() {
  while (!should_exit_) {
    pthread_mutex_lock(&mu_);
    while (queue_.empty() && !should_exit_) {
      pthread_cond_wait(&rsignal_, &mu_);
    }
    if (should_exit_) {
      pthread_mutex_unlock(&mu_);
      continue;
    }
    void (*function)(void*) = queue_.front().function;
    void* arg = queue_.front().arg;
    queue_.pop_front();
    pthread_cond_signal(&wsignal_);
    pthread_mutex_unlock(&mu_);
    (*function)(arg);
  }
  return NULL;
}
}
