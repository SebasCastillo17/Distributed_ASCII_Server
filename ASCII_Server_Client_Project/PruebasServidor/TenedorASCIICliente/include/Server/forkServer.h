# pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "VSocket.h"

struct FiguraServerInfo {
    std::string nombre;
    std::string ip;
    int puerto;
    std::vector<std::string> figuras;
};

class forkServer {
public:
    void init(uint32_t portClient, uint32_t portASCIIServer, char type = 's', bool ipv6 = false);
    
    std::string translateHttpToAscii(const char *httpRequest);
    std::string translateAsciiToHttp(const char *asciiResponse);
    std::string validateRequest(char *request);

private:
    void udpDiscoveryHandler();
    void monitorServers();
    std::string requestFromServer(const std::string& serverIp, int port, const std::string& request);
    void processProtocolMessage(const std::string& message, const std::string& senderIp);

    std::vector<FiguraServerInfo> servidoresRegistrados;
    std::map<std::string, std::pair<std::string, int>> knownServers;
    std::map<std::string, std::vector<std::string>> serverObjects;
    std::thread discoveryThread;
    std::thread monitorThread;
    std::mutex serverMutex;
    std::atomic<bool> running;
    std::string generateHTMLIndex();
    const int UDP_FORK_PORT = 4321;
    const int UDP_SERVER_PORT = 1234;
    const int BROADCAST_INTERVAL = 30;
    const int SERVER_TIMEOUT = 5;
    const int MAX_RETRIES = 3;
};
