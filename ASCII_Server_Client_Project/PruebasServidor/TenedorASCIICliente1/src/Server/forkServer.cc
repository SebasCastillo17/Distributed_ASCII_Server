#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "../../include/Server/Socket.h"
#include "../../include/Server/forkServer.h"

#define BUFFER 256
#define BC_ISLA "192.168.1.255"

void forkServer::init(uint32_t portClient, uint32_t portASCIIServer, char type,
                      bool ipv6) {
  running = true;

  discoveryThread = std::thread(&forkServer::udpDiscoveryHandler, this);

  monitorThread = std::thread(&forkServer::monitorServers, this);

  VSocket *Fork, *Client;
  Fork = new Socket(type, ipv6);

  Fork->Bind(portClient);
  Fork->MarkPassive(10);

  for (;;) {
    Client = Fork->AcceptConnection();

    if (!Client) {
      continue;
    }

    std::string httpRequest;
    char buffer[BUFFER];
    while (true) {
      memset(buffer, 0, BUFFER);
      int bytesRead = Client->Read(buffer, BUFFER);
      if (bytesRead <= 0)
        break;

      httpRequest.append(buffer, bytesRead);
      if (httpRequest.find("\r\n\r\n") != std::string::npos) {
        break;
      }
    }
    std::cout << httpRequest << std::endl;
    if (httpRequest.find("GET / ") != std::string::npos ||
        httpRequest.find("GET /HTTP") != std::string::npos) {
      std::string html = generateHTMLIndex();
      std::string httpResponse = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/html; charset=utf-8\r\n"
                                 "\r\n" +
                                 html;
      Client->Write(httpResponse.c_str(), httpResponse.size());
      Client->Close();
      continue;
    }
    std::string asciiRequest = translateHttpToAscii(httpRequest.c_str());

    std::cout << asciiRequest << std::endl;

    std::string asciiResponse;

    if (asciiRequest.find("BEGIN/OBJECTS/END") != std::string::npos) {
      std::lock_guard<std::mutex> lock(serverMutex);
      std::string objectList;
      for (const auto &server : serverObjects) {
        for (const auto &obj : server.second) {
          if (!objectList.empty())
            objectList += "\n";
          objectList += obj;
        }
      }
      asciiResponse = "BEGIN/OK/" + objectList + "/END";
    } else if (asciiRequest.find("BEGIN/GET/") != std::string::npos) {
      size_t start = asciiRequest.find("BEGIN/GET/") + 10;
      size_t end = asciiRequest.find("/END");
      if (end != std::string::npos) {
        std::string objectName = asciiRequest.substr(start, end - start);
        bool found = false;

        std::map<std::string, std::pair<std::string, int>> serversCopy;
        {
          std::lock_guard<std::mutex> lock(serverMutex);
          serversCopy = knownServers;
        }

        for (const auto &server : serversCopy) {
          const std::string &ip = server.second.first;
          int port = server.second.second;

          bool hasObject = false;
          {
            std::lock_guard<std::mutex> lock(serverMutex);
            if (serverObjects.count(ip)) {
              const auto &objects = serverObjects[ip];
              hasObject = std::find(objects.begin(), objects.end(),
                                    objectName) != objects.end();
            }
          }

          if (hasObject) {
            std::string getRequest = "BEGIN/GET/" + objectName + "/END";
            asciiResponse = requestFromServer(ip, port, getRequest);
            found = true;
            break;
          }
        }

        if (!found) {
          asciiResponse =
              "BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END";
        }
      }
    }
    std::cout << asciiResponse << std::endl;

    size_t sent = 0;

    std::string httpResponse = translateAsciiToHttp(asciiResponse.c_str());

    std::cout << httpResponse << std::endl;

    while (sent < httpResponse.size()) {
      size_t chunkSize = BUFFER < (httpResponse.size() - sent)
                             ? BUFFER
                             : (httpResponse.size() - sent);
      Client->Write(httpResponse.data() + sent, chunkSize);
      sent += chunkSize;
    }

    Client->Close();
  }
}

void forkServer::udpDiscoveryHandler() {
  try {
    Socket udpSocket('d', false);
    udpSocket.Bind(UDP_FORK_PORT);

    // Habilitar broadcast
    int broadcastEnable = 1;
    setsockopt(udpSocket.getSocketId(), SOL_SOCKET, SO_BROADCAST,
               &broadcastEnable, sizeof(broadcastEnable));

    std::string announceMsg = "BEGIN/ON/TENEDOR/END";
    sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(BC_ISLA);
    broadcastAddr.sin_port = htons(UDP_SERVER_PORT);
    std::cout << "[FORK] Buscando servidores por broadcast UDP..." << std::endl;

    udpSocket.sendTo(announceMsg.c_str(), announceMsg.size(), &broadcastAddr);

    char buffer[256];
    sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);

    while (running) {
      int bytes = recvfrom(udpSocket.getSocketId(), buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&senderAddr, &addrLen);
      if (bytes > 0) {
        std::string message(buffer, bytes);
        std::string senderIp = inet_ntoa(senderAddr.sin_addr);
        processProtocolMessage(message, senderIp);
      }
    }
  } catch (const std::runtime_error &e) {
    std::cerr << "Error en el hilo de descubrimiento UDP: " << e.what()
              << std::endl;
  } catch (...) {
    std::cerr << "Error en el hilo de descubrimiento UDP" << std::endl;
  }
}

void forkServer::processProtocolMessage(const std::string &message,
                                        const std::string &senderIp) {
  if (message.find("BEGIN/ON/SERVIDOR/") != std::string::npos) {
    size_t ipStart = message.find("BEGIN/ON/SERVIDOR/") + 18;
    size_t ipEnd = message.find('/', ipStart);
    size_t portEnd = message.find("/END");

    if (ipEnd != std::string::npos && portEnd != std::string::npos) {
      std::string ip = message.substr(ipStart, ipEnd - ipStart);
      int port = std::stoi(message.substr(ipEnd + 1, portEnd - (ipEnd + 1)));

      std::lock_guard<std::mutex> lock(serverMutex);
      knownServers[senderIp] = std::make_pair(ip, port);

      std::string objectsList =
          requestFromServer(ip, port, "BEGIN/OBJECTS/END");
      if (objectsList.find("BEGIN/OK/") != std::string::npos) {
        size_t contentStart = objectsList.find("BEGIN/OK/") + 9;
        size_t contentEnd = objectsList.find("/END");
        std::string content =
            objectsList.substr(contentStart, contentEnd - contentStart);

        std::vector<std::string> objects;
        std::istringstream iss(content);
        std::string object;
        while (std::getline(iss, object, '\n')) {
          objects.push_back(object);
        }

        serverObjects[ip] = objects;
      }
    }
  } else if (message.find("BEGIN/OFF/SERVIDOR/") != std::string::npos) {
    std::lock_guard<std::mutex> lock(serverMutex);
    knownServers.erase(senderIp);
    serverObjects.erase(senderIp);
  }
}

void forkServer::monitorServers() {
  while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(BROADCAST_INTERVAL));

    std::map<std::string, std::pair<std::string, int>> serversCopy;
    {
      std::lock_guard<std::mutex> lock(serverMutex);
      serversCopy = knownServers;
    }

    std::cout << "[FORK] Monitoreando servidores conocidos..." << std::endl;

    for (const auto &server : serversCopy) {
      const std::string &ip = server.second.first;
      int port = server.second.second;
      std::cout << "[FORK] Consultando al servidor " << ip << ":" << port
                << std::endl;
      bool active = false;
      for (int i = 0; i < MAX_RETRIES; ++i) {
        try {
          std::string response =
              requestFromServer(ip, port, "BEGIN/OBJECTS/END");
          if (response.find("BEGIN/OK/") != std::string::npos) {
            active = true;

            size_t contentStart = response.find("BEGIN/OK/") + 9;
            size_t contentEnd = response.find("/END");
            std::string content =
                response.substr(contentStart, contentEnd - contentStart);

            std::vector<std::string> objects;
            std::istringstream iss(content);
            std::string object;
            while (std::getline(iss, object, '\n')) {
              objects.push_back(object);
            }

            std::lock_guard<std::mutex> lock(serverMutex);
            serverObjects[ip] = objects;
            break;
          }
        } catch (...) {
          std::cout << "[FORK] Fallo al consultar " << ip << ":" << port
                    << ", reintentando..." << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }

      if (!active) {
        std::cout << "[FORK] El servidor " << ip << ":" << port
                  << " no responde, eliminando de la lista." << std::endl;
        std::lock_guard<std::mutex> lock(serverMutex);
        knownServers.erase(ip);
        serverObjects.erase(ip);
      }
    }
  }
}

std::string forkServer::requestFromServer(const std::string &serverIp, int port,
                                          const std::string &request) {
  try {
    Socket tcpSocket('s', false);

    // Configurar timeout
    struct timeval tv;
    tv.tv_sec = SERVER_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(tcpSocket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO, &tv,
               sizeof(tv));

    tcpSocket.MakeConnection(serverIp.c_str(), port);
    tcpSocket.Write(request.c_str());

    std::string response;
    char buffer[BUFFER];
    while (true) {
      int bytes = tcpSocket.Read(buffer, sizeof(buffer));
      if (bytes <= 0) {
        break;
      }
      response.append(buffer, bytes);
      // Detecta el final del mensaje
      if (response.find("/END") != std::string::npos) {
        break;
      }
    }

    if (response.empty()) {
      throw std::runtime_error("No response from server");
    }

    return response;
  } catch (...) {
    throw std::runtime_error("Failed to connect to server");
  }
}

std::string forkServer::translateHttpToAscii(const char *httpRequest) {
  std::string request(httpRequest);

  if (request.find("GET /objects") != std::string::npos) {
    return "BEGIN/OBJECTS/END";
  } else if (request.find("GET /get/") != std::string::npos) {
    size_t start = request.find("GET /get/") + 9;
    size_t end = request.find(" HTTP/");
    if (end != std::string::npos) {
      std::string asciiName = request.substr(start, end - start);
      return "BEGIN/GET/" + asciiName + "/END";
    }
  } else if (request.find("POST /on/server") != std::string::npos) {
    return "BEGIN/ON/SERVIDOR/param1/param2/END";
  } else if (request.find("POST /on/fork") != std::string::npos) {
    return "BEGIN/ON/TENEDOR/END";
  } else if (request.find("POST /off/server") != std::string::npos) {
    return "BEGIN/OFF/SERVIDOR/param1/param2/END";
  }

  return "BEGIN/ERROR/400/Invalid HTTP request/END";
}

std::string forkServer::translateAsciiToHttp(const char *asciiResponse) {
  std::string response(asciiResponse);
  if (response.find("BEGIN/ERROR/") != std::string::npos) {
    size_t codeStart = response.find("BEGIN/ERROR/") + 12;
    size_t codeEnd = response.find('/', codeStart);
    std::string code = response.substr(codeStart, codeEnd - codeStart);
    std::string message =
        response.substr(codeEnd + 1, response.length() - codeEnd - 5);

    return "HTTP/1.1 " + code +
           " Error\r\n"
           "Content-Type: text/plain\r\n"
           "\r\n" +
           message;
  }
  if (response.find("BEGIN/OK/") != std::string::npos) {
    std::string content = response.substr(9, response.length() - 13);

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain; charset=utf-8\r\n"
           "\r\n" +
           content;
  }
  return "HTTP/1.1 500 Internal Server Error\r\n"
         "Content-Type: text/plain\r\n"
         "\r\nInvalid server response format";
}
std::string forkServer::validateRequest(char *request) { return ""; }

std::string forkServer::generateHTMLIndex() {
  std::lock_guard<std::mutex> lock(serverMutex);
  std::string html =
      "<html><head><title>Servidores de Figuras</title></head><body>";
  html += "<h1>Servidores Registrados</h1>";
  html += "<table border='1'><tr><th>Servidor "
          "(IP:Puerto)</th><th>Figuras</th></tr>";
  for (const auto &entry : serverObjects) {
    const std::string &ip = entry.first;
    std::string port = "???";
    // Busca el puerto en knownServers
    for (const auto &kv : knownServers) {
      if (kv.second.first == ip) {
        port = std::to_string(kv.second.second);
        break;
      }
    }
    html += "<tr><td>" + ip + ":" + port + "</td><td><ul>";
    for (const auto &fig : entry.second) {
      html += "<li>" + fig + "</li>";
    }
    html += "</ul></td></tr>";
  }
  html += "</table></body></html>";
  return html;
}
