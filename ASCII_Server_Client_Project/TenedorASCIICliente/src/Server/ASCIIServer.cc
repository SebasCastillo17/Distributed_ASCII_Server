#include "../../include/Server/ASCIIServer.h"
#include "../../include/FileSystem/FileSystem.h"
#include "../../include/Server/Socket.h"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <chrono>
#include <arpa/inet.h>

#define BUFFER 256
#define BC_ISLA "192.168.1.255"

void *handleRequestWrapper(void *arg) {
    auto params = static_cast<std::pair<ASCIIServer *, VSocket *> *>(arg);
    params->first->handleRequest(params->second);
    params->second->Close();
    delete params;
    return nullptr;
}

void ASCIIServer::init(uint32_t port, uint32_t maxConnection, char type, bool ipv6) {
    this->port = port;
    running = true;
    
    // Iniciar hilo de broadcast
    broadcastThread = std::thread(&ASCIIServer::udpBroadcastHandler, this);
    
    // Configuración TCP original
    VSocket *server, *client;
    server = new Socket(type, ipv6);
    server->Bind(port);
    server->MarkPassive(maxConnection);

    for (;;) {
        client = server->AcceptConnection();
        auto *params = new std::pair<ASCIIServer *, VSocket *>(this, client);
        pthread_t thread;
        if (pthread_create(&thread, nullptr, handleRequestWrapper, params) != 0) {
            perror("Failed to create thread");
            delete params;
            client->Close();
        }
        pthread_detach(thread);
    }
}

void ASCIIServer::udpBroadcastHandler() {
    try {
        Socket udpSocket('d', false);
        udpSocket.Bind(UDP_SERVER_PORT);
        std::cout << "[ASCIIServer] Enviando anuncio inicial de presencia a tenedores..." << std::endl;
        sendUdpAnnouncement();
        
        char buffer[256];
        sockaddr_in senderAddr;
        socklen_t addrLen = sizeof(senderAddr);
        
        while (running) {
            int bytes = recvfrom(udpSocket.getSocketId(), buffer, sizeof(buffer), 0,
                              (struct sockaddr*)&senderAddr, &addrLen);
            if (bytes > 0) {
                std::string message(buffer, bytes);
                std::string senderIp = inet_ntoa(senderAddr.sin_addr);
                if (message.find("BEGIN/ON/TENEDOR") != std::string::npos) {
                    std::cout << "[ASCIIServer] Tenedor detectado en " << senderIp << ", respondiendo con anuncio." << std::endl;
                    // Un tenedor se ha anunciado, responder con nuestro anuncio
                    sendUdpAnnouncement();
                }
            }
            
            // Enviar anuncios periódicos
            std::this_thread::sleep_for(std::chrono::seconds(BROADCAST_INTERVAL));
            if (running) {
                std::cout << "[ASCIIServer] Enviando anuncio periódico de presencia a tenedores..." << std::endl;
                sendUdpAnnouncement();
            }
        }
    } catch (...) {
        std::cerr << "Error en el hilo de broadcast UDP" << std::endl;
    }
}

void ASCIIServer::sendUdpAnnouncement(bool offline) {
    try {
        Socket udpSocket('d', false);
        int broadcastEnable = 1;
        setsockopt(udpSocket.getSocketId(), SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
        
        std::string localIP = "127.0.0.1"; // Simplificado, debería detectar la IP real
        std::string message;
        
        if (offline) {
            message = "BEGIN/OFF/SERVIDOR/" + localIP + "/" + std::to_string(port) + "/END";
            std::cout << "[ASCIIServer] Anunciando que el servidor está OFFLINE a los tenedores." << std::endl;
        } else {
            message = "BEGIN/ON/SERVIDOR/" + localIP + "/" + std::to_string(port) + "/END";
            std::cout << "[ASCIIServer] Anunciando que el servidor está ONLINE a los tenedores." << std::endl;
        }
        
        sockaddr_in broadcastAddr;
        memset(&broadcastAddr, 0, sizeof(broadcastAddr));
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_addr.s_addr = inet_addr(BC_ISLA);
        broadcastAddr.sin_port = htons(4321);
        
        udpSocket.sendTo(message.c_str(), message.size(), &broadcastAddr);
    } catch (...) {
        std::cerr << "Error al enviar anuncio UDP" << std::endl;
    }
}

void ASCIIServer::handleRequest(void *socket) {
    VSocket *clientSocket = (VSocket *)socket;
    char request[BUFFER];
    memset(request, 0, BUFFER);
    clientSocket->Read(request, BUFFER);
    std::string response = validateRequest(request);
    
    size_t sent = 0;
    while (sent < response.size()) {
        size_t chunkSize = BUFFER < (response.size() - sent) ? BUFFER : (response.size() - sent);
        std::cout << std::endl;
        std::cout << response.substr(sent, chunkSize).c_str() << std::endl;
        std::cout << std::endl;
        clientSocket->Write(response.substr(sent, chunkSize).c_str());
        sent += chunkSize;
    }
}

std::string ASCIIServer::validateRequest(char *request) {
    if (!request || strlen(request) == 0) {
        return "BEGIN/ERROR/300/El mensaje está vacío o es inválido/END";
    }

    std::string req(request);

    if (req.substr(0, 6) != "BEGIN/" || req.substr(req.length() - 4) != "/END") {
        return "BEGIN/ERROR/300/Formato de mensaje inválido/END";
    }

    std::string content = req.substr(6, req.length() - 10);
    std::vector<std::string> tokens;
    size_t pos = 0;

    while ((pos = content.find('/')) != std::string::npos) {
        tokens.push_back(content.substr(0, pos));
        content.erase(0, pos + 1);
    }
    tokens.push_back(content);

    if (tokens.empty()) {
        return "BEGIN/ERROR/300/Mensaje mal formateado/END";
    }

    if (tokens.size() == 1 && tokens[0] == "OBJECTS") {
        std::string objectList = (std::string)getSystemInfo();
        return "BEGIN/OK/" + objectList + "/END";
    }

   if (tokens.size() == 2 && tokens[0] == "GET") {
    std::string ascii = tokens[1];
    if (!existASCII((char *)ascii.c_str())) {
      std::string contenido =
          "Este es el contenido del dibujo " + ascii + ":\n";
      std::string asciiArt = "";
      uint8_t next = 0;
      while (next != 255) {
        char *block = extractBlock((char *)ascii.c_str(), next);
        next = (uint8_t)block[255];
        block[255] = '\0';
        asciiArt += (std::string)block;
        delete block;
      }
      std::string response = "BEGIN/OK/" + contenido + asciiArt + "/END";
      return response;
    } else {
      return "BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END";
    }
  }

    return "BEGIN/ERROR/300/Mensaje no reconocido por el protocolo/END";
}

