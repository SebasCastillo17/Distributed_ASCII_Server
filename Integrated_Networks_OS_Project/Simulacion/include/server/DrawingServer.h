// Copyright [2025] <AIGroup>

#pragma once

#include <common/Thread.h>
#include <string>

# include "ColabProtocolHandler.h"

class DrawingServer : public Thread {
 public:
  DrawingServer() = default;
  void *run(void *);
  static void *runDrawingServer(void *arg);  // Método estático como envoltorio

 protected:
  ColabProtocolHandler GCP;
};
