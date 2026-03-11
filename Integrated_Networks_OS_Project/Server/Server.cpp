#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>	// memset
#include <vector>
#include <sstream>

#include "Server.h"

// Constructor por defecto: usa IP y puerto predeterminados, sin usar hilos
Server::Server(bool usarHilos) {
    this->ip = IP;
    this->port = PORT;
    this->usarHilos = usarHilos;
}

// Constructor con parámetros de IP y puerto
Server::Server(const char * ip, int port, bool usarHilos) {
    this->ip = ip;
    this->port = port;
    this->usarHilos = usarHilos;
}

// Otro constructor por defecto (sin hilos)
Server::Server() {
    this->ip = IP;
    this->port = PORT;
    this->usarHilos = false;
}

// Destructor
Server::~Server() {
}

// Monta el sistema de archivos cargando archivos de texto al sistema virtual
void Server::MountSystem() {
    mountSystem();
    writeFile((char *)"Goku.txt", (char *)"../../Objetos/Goku.txt");
    writeFile((char *)"Gogeta.txt", (char *)"../../Objetos/Gogeta.txt");
    writeFile((char *)"Foca.txt", (char *)"../../Objetos/Foca.txt");
    writeFile((char *)"CocaCola.txt", (char *)"../../Objetos/CocaCola.txt");
    writeFile((char *)"Exitante.txt", (char *)"../../Objetos/Exitante.txt");
    writeFile((char *)"GoyoCat.txt", (char *)"../../Objetos/GoyoCat.txt");

    std::cout << "Sistema de archivos montado" << std::endl;
}   

/**
 * Tarea que ejecutará cada nuevo hilo:
 *   - Leer cadena del socket
 *   - Procesarla y responder al cliente
 */
void task(Server *server, VSocket *client) {

    std::string httpRequest;
    char buffer[BUFSIZE];
    while (true) {
        memset(buffer, 0, BUFSIZE);  // Limpia el buffer
        int bytesRead = client->Read(buffer, BUFSIZE);  // Lee del cliente
        if (bytesRead <= 0)
            break;

        httpRequest.append(buffer, bytesRead);  // Agrega lo leído a la solicitud
        if (httpRequest.find("\r\n\r\n") != std::string::npos) {
            break;  // Fin de la cabecera HTTP
        }
    }
    std::cout << httpRequest << std::endl;

    std::string asciiRequest = server->translateHttpToAscii(httpRequest.c_str());  // Traduce a formato ASCII interno
    std::cout << "ASCII Request: " << asciiRequest << std::endl;
    std::string asciiResponse = server->handleRequest(client, asciiRequest.c_str());  // Maneja la solicitud

    size_t sent = 0;
    while (sent < asciiResponse.size()) {
        size_t chunkSize = BUFSIZE < (asciiResponse.size() - sent) ? BUFSIZE : (asciiResponse.size() - sent);
        std::cout << std::endl << asciiResponse.substr(sent, chunkSize).c_str() << std::endl << std::endl;
        client->Write(asciiResponse.substr(sent, chunkSize).c_str());  // Envía la respuesta por partes
        sent += chunkSize;
    }
    std::cout << "Response sent" << std::endl;
    client->Close();  // Cierra el socket del cliente
}

// Método principal que corre el servidor
void Server::run(){

    this->MountSystem();  // Monta el sistema de archivos
    std::cout << "Servidor escuchando con la ip: " << this->ip << " en el puerto: " << this->port << std::endl;

    if(!this->usarHilos){
        std::cout << "Modo Hilos" << std::endl;

        this->serverSocket = new Socket('s');	// Crea un socket de tipo stream (IPv4)
        this->serverSocket->Bind(PORT);		// Enlaza el puerto
        this->serverSocket->MarkPassive(5);	// Espera conexiones

        for( ; ; ) {
            VSocket* client = this->serverSocket->AcceptConnection();	 // Acepta conexión

            // Lanza un hilo independiente para manejar el cliente
            std::thread([this, client]() {
                task(this, client);
            }).detach();
        }

    }
    else {
        std::cout << "Modo Procesos" << std::endl;

        this->serverSocket = new Socket('s');	// Crea socket tipo stream
        int childpid;
        char a[BUFSIZE];

        this->serverSocket->Bind(PORT);		// Enlaza puerto
        this->serverSocket->MarkPassive(5);	// Espera conexioness

        for( ; ; ) {
            VSocket* client = this->serverSocket->AcceptConnection();	// Acepta conexión
            childpid = fork();		// Crea proceso hijo
            if (childpid < 0) {
               perror("server: fork error");
            } else {
               if (0 == childpid) {		// Código del hijo
                    this->serverSocket->Close();	// Cierra socket original en hijo
                    memset(a, 0, BUFSIZE);
                    client->Read(a, BUFSIZE);	// Lee del cliente
                    client->Write(a);		// Devuelve el mismo mensaje
                    exit(0);			// Termina hijo
               }
            }
            client->Close();	// El padre cierra el socket cliente
         }
    }
}

// Maneja la solicitud traducida
std::string Server::handleRequest(VSocket *socket, const char *request) {
    VSocket *Server = socket;
    std::cout << "Request: " << request << std::endl;
    std::string response = validateRequest(request);  // Valida el mensaje interno
    std::string responseHttp = this->translateAsciiToHttp(response.c_str());  // Traduce a HTTP
    return responseHttp;
}

// Valida el contenido del mensaje en formato interno ASCII
std::string Server::validateRequest(const char *request) {
    std::string asciiList = "CocaCola.txt, Exitante.txt, Foca.txt, Gogeta.txt, "
                            "Goku.txt, GoyoCat.txt, Personal.txt";

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

    // Lista todos los objetos
    if (tokens.size() == 1 && tokens[0] == "OBJECTS") {
      std::string response = "BEGIN/OK/" + asciiList + "/END";
      return response;
    }

    // Obtiene un objeto específico
    if (tokens.size() == 2 && tokens[0] == "GET") {
      std::string ascii = tokens[1] + ".txt";
      if (!existASCII((char *)ascii.c_str())) {
        std::string contenido = "Este es el contenido del dibujo " + ascii + ":\n";
        std::string asciiArt = "";
        uint8_t next = 0;
        while (next != 255) {
          char *block = extractBlock((char *)ascii.c_str(), next);
          next = (uint8_t)block[255];
          block[255] = '\0';
          asciiArt += (std::string)block;
          delete block;
        }
        std::string response = "BEGIN/OK/" + asciiArt + "/END";
        return response;
      } else {
        return "BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END";
      }
    }

    // Registro de servidor o tenedor
    if (tokens.size() >= 2 && tokens[0] == "ON") {
      if (tokens[1] == "SERVIDOR" && tokens.size() == 4) {
        return "BEGIN/OK/Servidor registrado/END";
      } else if (tokens[1] == "TENEDOR") {
        return "BEGIN/OK/Tenedor registrado/END";
      }
    }

    // Desconexión del servidor
    if (tokens.size() >= 2 && tokens[0] == "OFF") {
      if (tokens[1] == "SERVIDOR" && tokens.size() == 4) {
        return "BEGIN/OK/Servidor desconectado/END";
      }
    }

    return "BEGIN/ERROR/300/Mensaje no reconocido por el protocolo/END";
}

// Traduce una solicitud HTTP a formato ASCII interno
std::string Server::translateHttpToAscii(const char *httpRequest) {
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

// Traduce una respuesta ASCII interna a formato HTTP
std::string Server::translateAsciiToHttp(const char *asciiResponse) {
  std::string response(asciiResponse);

  if (response.find("BEGIN/ERROR/") != std::string::npos) {
      size_t codeStart = response.find("BEGIN/ERROR/") + 12;
      size_t codeEnd = response.find('/', codeStart);
      std::string code = response.substr(codeStart, codeEnd - codeStart);
      std::string message = response.substr(codeEnd + 1, response.length() - codeEnd - 5);

      std::ostringstream http;
      http << "HTTP/1.1 " << code << " Error\r\n"
           << "Content-Type: text/plain\r\n"
           << "Content-Length: " << message.size() << "\r\n"
           << "Connection: close\r\n"
           << "\r\n"
           << message;
      return http.str();
  }

  if (response.find("BEGIN/OK/") != std::string::npos) {
      std::string content = response.substr(9, response.length() - 13);

      std::ostringstream http;
      http << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: text/plain; charset=utf-8\r\n"
           << "Content-Length: " << content.size() << "\r\n"
           << "Connection: close\r\n"
           << "\r\n"
           << content;
      return http.str();
  }

  std::string errorMsg = "Invalid server response format";
  std::ostringstream http;
  http << "HTTP/1.1 500 Internal Server Error\r\n"
       << "Content-Type: text/plain\r\n"
       << "Content-Length: " << errorMsg.size() << "\r\n"
       << "Connection: close\r\n"
       << "\r\n"
       << errorMsg;
  return http.str();
}