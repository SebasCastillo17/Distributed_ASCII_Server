// Copyright [2025] <AIGroup>

#pragma once

#include <map>
#include <string>

#include <common/Thread.h>
#include "RequestColabHTTPAdapter.h"

class Tenedor : public Thread {
 public:
  Tenedor() = default;
  void *run(void *);
  static void *runTenedor(void *arg);  // Método estático como envoltorio
 private:
  RequestColabHTTPAdapter requestColabHTTPAdapter;
  std::string validateRequest(std::string request);
  std::string validateResponse(std::string response);
  std::map<std::string, std::string> AIProtocol = {
      {"GET /list HTTP/1.1", "get/home"},
      {"GET /Gogeta.txt HTTP/1.1", "get/Gogeta.txt"},
      {"GET /Goku.txt HTTP/1.1", "get/Goku.txt"},
      {"GET /GoyoCat.txt HTTP/1.1", "get/GoyoCat.txt"},
  };
  std::map<std::string, std::string> HTTP = {
      {"get/home", "ok 200"},
      {"[Gogeta imagen]", "ok 200"},
      {"[Goku imagen]", "ok 200"},
      {"[GoyoCat imagen]", "ok 200"},
      {"List: Gogeta.txt, Goku.txt, GoyoCat.txt", "ok 200"}};
  void getFiguresProtocol();
};
