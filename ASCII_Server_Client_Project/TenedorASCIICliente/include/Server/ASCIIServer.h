# pragma once

#include "Server.h"
#include "VSocket.h"

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class ASCIIServer : public Server {
 public:
    void init(uint32_t port, uint32_t maxConnection, char type = 's', bool ipv6 = false);
    void handleRequest(void *socket);
    std::string validateRequest(char *request);

 private:
    void udpBroadcastHandler();
    void sendUdpAnnouncement(bool offline = false);
    
    std::map<std::string, std::string> books; // Nombre -> contenido
    std::thread broadcastThread;
    std::mutex booksMutex;
    std::atomic<bool> running;
    
    // Constantes del protocolo
    const int UDP_SERVER_PORT = 1234;
    const int BROADCAST_INTERVAL = 30;
    uint32_t port;
};

