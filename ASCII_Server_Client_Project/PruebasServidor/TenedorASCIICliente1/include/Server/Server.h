#pragma once

#include <cstdint>

#include "../../include/Server/Server.h"
#include "Socket.h"
#include "VSocket.h"

class Server {
 public:

  /**
   * @brief Start the server
   *
   * @param port to connect the port of the server
   */
  void startSocket(uint32_t port, uint32_t maxConnection, char type = 's', bool ipv6 = false);

  ~Server() = default;
  
 protected:
};

