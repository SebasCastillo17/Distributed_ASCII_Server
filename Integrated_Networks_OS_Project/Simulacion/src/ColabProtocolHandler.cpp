#include "server/ColabProtocolHandler.h"
#include "common/ColabUtility.h"
#include "server/FileSystem/FileSystem.h"
#include <stdexcept>

ColabProtocolHandler::ColabProtocolHandler() {
  if (mountSystem() == EXIT_FAILURE) {
    throw std::runtime_error("Error mounting the filesystem");
  }

  writeFile((char *)"Personal.txt", (char *)"ascii/Personal.txt");
  writeFile((char *)"Goku.txt", (char *)"ascii/Goku.txt");
  writeFile((char *)"Gogeta.txt", (char *)"ascii/Gogeta.txt");
  writeFile((char *)"Foca.txt", (char *)"ascii/Foca.txt");
  writeFile((char *)"CocaCola.txt", (char *)"ascii/CocaCola.txt");
  writeFile((char *)"Exitante.txt", (char *)"ascii/Exitante.txt");
  writeFile((char *)"GoyoCat.txt", (char *)"ascii/GoyoCat.txt");

};

std::string ColabProtocolHandler::buildAnswer(std::string request) {
  std::string response = this->deleteFlags(request);
  if (response == "") {
    return "BEGIN/ERROR/200/END";
  }

  if (response == "OBJECTS") {
    return this->objCase();
  }

  size_t pos = response.find('/');
  if (pos == std::string::npos) {
    return "BEGIN/ERROR/300/END";
  }

  std::string operationString = response.substr(0, pos);
  int operationCase = this->determOperation(operationString);

  if (operationCase == -1) {
    return "BEGIN/ERROR/300/END";
  }

  response = response.substr(pos + 1);

  switch (operationCase) {
    case 1:
      this->onCase(response);
      break;
    case 2:
      this->offCase(response);
      break;
    case 3:
      this->okCase(response);
      break;
    case 4:
      this->errorCase(response);
      break;
    case 5:
      response = this->getCase(response);
      if (response == "-1") {
        return "BEGIN/ERROR/201/END";
      }
      break;
  }

  return response;
}

std::string ColabProtocolHandler::deleteFlags(std::string request) {
  std::string init = FP;
  std::string end = LP;

  size_t initPos = request.find(FP); 
  size_t endPos = request.find(LP);

  if (initPos == std::string::npos || endPos == std::string::npos) {
    return "";
  }

  initPos += init.length();  // Skip "BEGIN/"
  std::string resultado = request.substr(initPos, endPos - initPos);  // {information}
  return resultado;
}

int ColabProtocolHandler::determOperation(std::string request) {
  if (this->OPP.find(request) != this->OPP.end()) {
    return this->OPP[request];
  } else {
    return -1;
  }
}

std::string ColabProtocolHandler::onCase(std::string service) {
  size_t serviceType = service.find('/');
  if (serviceType == std::string::npos) {
    return "Error, solicitud ON inválida\n";
  }
  return std::string();
}

std::string ColabProtocolHandler::objCase() {
  return "BEGIN/OK/" + this->nameList + "/END";
}

std::string ColabProtocolHandler::getCase(std::string request) {
  if (!existASCII((char *)request.c_str())) {
    std::string asciiArt = "";
    uint8_t next = 0;
    while (next != 255) {
      char *block = extractBlock((char *)request.c_str(), next);
      next = (uint8_t)block[255];
      block[255] = '\0';
      asciiArt += (std::string)block;
      delete block;
    }
    std::string response = "BEGIN/OK/" + asciiArt + "/END";
    return response;
  } else {
    printf("No existe el archivo\n");
    return "-1";
  }
}

std::string ColabProtocolHandler::offCase(std::string request) {
  // Implementación de ejemplo
  return "OFF case handled: " + request;
}

std::string ColabProtocolHandler::okCase(std::string request) {
  // Implementación de ejemplo
  return "OK case handled: " + request;
}

std::string ColabProtocolHandler::errorCase(std::string request) {
  // Implementación de ejemplo
  return "ERROR case handled: " + request;
}