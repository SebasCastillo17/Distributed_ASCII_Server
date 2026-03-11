// Copyright [2025] <AIGroup>

#pragma once

#include <common/Thread.h>
#include <string>

class Client : public Thread {
 public:
  Client() = default;
  void *run(void *);
  // Wrapper
  static void *runClient(void *arg);

 private:
  int validateRequest(std::string request);
  std::string REQUEST[4] = {"GET /list HTTP/1.1", "GET /Gogeta.txt HTTP/1.1",
                            "GET /Goku.txt HTTP/1.1",
                            "GET /GoyoCat.txt HTTP/1.1"};
};
