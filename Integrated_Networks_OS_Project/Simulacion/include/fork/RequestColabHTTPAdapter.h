// RequestAdapter.h
// Adaptador para manejar las solicitudes de los clientes y convertirlas en
// mensajes para el servidor
#pragma once 
#include <string>
#include <vector>
#include "common/ErrorTableColab.h"
#include <algorithm>
#include <stdexcept>


class RequestColabHTTPAdapter {
 public:
  RequestColabHTTPAdapter() = default;
  ~RequestColabHTTPAdapter() = default;

  // Método para construir la solicitud Colab si el request es válido
  // ó respuesta HTTP si el request no es válido
  int buildRequest(const std::string &request, std::string &response) {
    // Se obtiene la primera linea del request ej: GET /object/tenedor.txt HTTP/1.1
    std::string firstLine = request;
    // Se valida que sea una solicitud GET
    if (firstLine.find("GET ") != 0 || firstLine.rfind("HTTP/1.1") != firstLine.length() - 8) {
      // Si no es una solicitud GET, se construye la respuesta HTTP
      response = httpBadRequest();
      return EXIT_FAILURE; // Código de error para solicitud inválida
    }

    // Se obtiene el contenido de la solicitud
    size_t start = 5; // después de "GET "
    size_t end = firstLine.rfind("HTTP/1.1");
    if (end == std::string::npos || end <= start) {
        response = httpBadRequest();
        return EXIT_FAILURE;
    }
    std::string content = firstLine.substr(start, end - start - 1);  // -1 para eliminar el espacio antes de HTTP/1.1
    printf("Content: %s\n", content.c_str());
    // Splitear contenido por cada /
    if (!content.empty() && content[0] == '/') content.erase(0, 1); // Eliminar el primer /
    std::vector<std::string> contentSplitted;
    size_t pos = 0;
    while ((pos = content.find('/')) != std::string::npos) {
      contentSplitted.push_back(content.substr(0, pos));
      content.erase(0, pos + 1);
    }

    if (!content.empty()) {
      contentSplitted.push_back(content); // Agregar el último objeto
    } else {
      printf("Error1: %s\n", content.c_str());
      response = httpBadRequest(); // Si no hay contenido, se construye la respuesta HTTP
      return EXIT_FAILURE; // Código de error para solicitud inválida
    }
    printf("Content Splitted: ");
    for (const auto &c : contentSplitted) {
      printf("%s ", c.c_str());
    }
    switch (contentSplitted.size()) {
    case 1:
      // Si solo hay una palabra, solo debería pedir objects
      if (contentSplitted[0] == "objects") {
        // Se construye la solicitud Colab
        response = httpObtenerObjects();
        return EXIT_FAILURE; // Código de éxito
      } else {
        // Si no es un objeto válido, se construye la respuesta HTTP
        response = httpBadRequest();
        return EXIT_FAILURE; // Código de error para solicitud inválida
      }

    case 3:
      // Si hay dos palabras, se debería pedir un objeto
      if (contentSplitted[0] == "object") {
        // Se trata de pasar el segundo valor a entero
        int objectId;
        try {
            objectId = std::stoi(contentSplitted[1]);
        } catch (const std::invalid_argument& e) {
            // Si no se puede convertir a entero, se construye la respuesta HTTP
            response = httpBadRequest();
            return EXIT_FAILURE; // Código de error para solicitud inválida
        } catch (const std::out_of_range& e) {
            // Si el número está fuera del rango de un entero, se construye la respuesta HTTP
            response = httpBadRequest();
            return EXIT_FAILURE; // Código de error para solicitud inválida
        }
        // Se trata de buscar el servidor y el objeto
        std::string objectName = contentSplitted[2];

        auto it = std::find_if(serverFigures.begin(), serverFigures.end(),
            [objectId, &objectName](const std::pair<int, std::vector<std::string>>& figure) {
                if (figure.first != objectId) return false;
                return std::find(figure.second.begin(), figure.second.end(), objectName) != figure.second.end();
            });

      if (it != serverFigures.end()) {
        // Si se encuentra el objeto, se construye la solicitud Colab
        response = "BEGIN/GET/" + objectName + "/END";
        return EXIT_SUCCESS; // Código de éxito
      } else {
        // Si no se encuentra el objeto, se construye la respuesta HTTP
        response = httpBadRequest();
        return EXIT_FAILURE; // Código de error para solicitud inválida
      }

      } else {
        // Si no es un objeto válido, se construye la respuesta HTTP
        response = httpBadRequest();
        return EXIT_FAILURE; // Código de error para solicitud inválida
      }
      break;
    
    default:
      // Si no es un objeto válido, se construye la respuesta HTTP
      response = httpBadRequest();
      return EXIT_FAILURE; // Código de error para solicitud inválida
      break;
    }
  
    return EXIT_FAILURE; // Código de error para solicitud inválida
  }

  // Método para construir la respuesta HTTP en base a la solicitud Colab
  std::string buildResponse(const std::string &request) {
    // String para devolver respuesta
    std::string response;
    // Llamar al método para obtener el contenido de la solicitud
    // y construir la respuesta HTTP
    if (!getContent(request, response)) return response;

    // Splitear para obtener comando y resto

    std::pair<std::string, std::string> commandRest = getCommandAndRest(response);

    return checkFirstInput(commandRest);
  }


  std::string getFiguresProtocol() { return "BEGIN/OBJECTS/END"; }

  void setServerFiguresByResponse(const std::string &response) {
    std::string content;

    if (!getContent(response, content)) {
        printf("Error: %s\n", content.c_str());
        return;
    }
    printf("Content pre: %s\n", content.c_str());
    // Obtener comando y resto del contenido
    std::pair<std::string, std::string> commandRest = getCommandAndRest(content);

    printf("Command: %s\n", commandRest.first.c_str());

    if (commandRest.first == "OK") {
        std::string objects = commandRest.second;
        std::vector<std::string> objectsList;
        size_t pos = 0;

        while ((pos = objects.find('\n')) != std::string::npos) {
            std::string obj = objects.substr(0, pos);
            if (!obj.empty()) objectsList.push_back(obj);
            objects.erase(0, pos + 1);
        }

        if (!objects.empty()) objectsList.push_back(objects); // Último objeto

        // Guardar en serverFigures
        serverFigures.push_back({serverFigures.size(), objectsList});
        printf("Se han agregado %zu objetos al tenedor\n", objectsList.size());
    } else {
        printf("Comando no reconocido: %s\n", commandRest.first.c_str());
    }
}

 private:
  // Vector para manejar los objetos disponibles
  std::vector<std::pair<int, std::vector<std::string>>> serverFigures;


  // Metodo para dividir string en dos partes significativas
  std::pair<std::string, std::string> getCommandAndRest(const std::string& response) {
    size_t pos = response.find('/');
    if (pos != std::string::npos) {
        std::string comando = response.substr(0, pos);
        std::string resto = response.substr(pos + 1);
        return {comando, resto};
    }
    return {"", ""}; // Por si no hay "/"
  }

  std::string checkFirstInput(std::pair<std::string, std::string> commandRest) {

    if (commandRest.first == "OK") {
        return httpFigure(commandRest.second);
    } else if (commandRest.first == "ERROR") {
        return httpErrorCode(commandRest.second);
    } else {
        return httpErrorFromServer();
    }
  }

  std::string httpFigure(const std::string &rest) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(rest.length()) + "\r\n";
    response += "\r\n";
    response += rest;
    return response;
  }

  std::string httpErrorCode(const std::string &rest) {
    std::string response = "HTTP/1.1 500 Internal Server Error\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(rest.length()) + "\r\n";
    response += "\r\n";
    response += rest;
    return response;
  }

  std::string httpErrorFromServer() {
    std::string response = "HTTP/1.1 Unknown Server Error\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: 0\r\n";
    response += "\r\n";
    return response;
  }

  std::string httpBadRequest() {
    std::string response = "HTTP/1.1 400 Bad Request\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: 0\r\n";
    response += "\r\n";
    return response;
  }

  std::string httpObtenerObjects() {
    std::string figures;
    std::string server;
    for (const auto &figure : serverFigures) {
      server = std::to_string(figure.first);
        for (const auto &object : figure.second) {
            figures += server + "/" + object + "\n";
        }
    }
    std::cout << "Server figures: " << figures << std::endl;
    // Devolver identificacion y figuras disponibles
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(figures.length()) + "\r\n";
    response += "\r\n";
    response += figures;
    return response;
  }

  // Método para obtener el contenido de la solicitud
  // y construir la respuesta HTTP
  // Retorna true si la solicitud es válida y 
  // construye solicitud Colab para el servidor
  // Retorna false si la solicitud no es válida y 
  // construye respuesta HTTP para el cliente
bool getContent(const std::string &request, std::string &response) {
    // Validar formato: debe comenzar con "GET/" y terminar con "/END"
    if (request.rfind("BEGIN/") != 0 || request.rfind("/END") != request.length() - 4) {
        response = "HTTP/1.1 400 Bad Request\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";
        return false;
    }

    // Extraer el contenido entre "GET/" y "/END"
    size_t start = 6; // después de "GET/"
    size_t end = request.rfind("/END");

    if (end <= start) {
        response = "HTTP/1.1 400 Bad Request\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";
        return false;
    }

    response = request.substr(start, end - start);
    return true;
}

  // Método para obtener el contenido de la solicitud
  // y construir la respuesta HTTP
  // Retorna true si la solicitud es válida y 
  // construye solicitud Colab para el servidor
  // Retorna false si la solicitud no es válida y 
  // construye respuesta HTTP para el cliente
  bool getContentList(const std::string &request, std::string &response) {
    // Validar formato: debe comenzar con "GET/" y terminar con "/END"
    if (request.find("GET/") != 0 || request.rfind("/END") != request.length() - 4) {
        printf("Formato inválido1: %s\n", request.c_str());
        return false;
    }

    // Extraer el contenido entre "GET/" y "/END"
    size_t start = 5; // después de "GET/"
    size_t end = request.rfind("/END");

    if (end <= start) {
        printf("Formato inválido2: %s\n", request.c_str());
        return false;
    }

    printf("Request: %s\n", request.c_str());
    response = request.substr(start, request.length() - 4);
    printf("Response: %s\n", response.c_str());
    return true;
}

};

