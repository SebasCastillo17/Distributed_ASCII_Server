// Copyright [2025] <AIGroup>

#pragma once

#include <pthread.h>

class Thread {
 public:
  ~Thread() = default;
  pthread_t OwnThread;

 protected:
  virtual void *run(void *) = 0;
};
