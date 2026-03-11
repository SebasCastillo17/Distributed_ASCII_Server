// Copyright [2025] <AIGroup>

#include <common/SharedMemory.h>
#include <fork/Tenedor.h>
#include <iomanip>

void *Tenedor::run(void *data) {
  SharedMemory *sharedMemory = static_cast<SharedMemory *>(data);
  std::string requestFigures = this->requestColabHTTPAdapter.getFiguresProtocol();
  char *request = (char *)requestFigures.c_str(); //NOLINT

  sharedMemory->write(request);
  std::cout << "Mensaje enviado al servidor: " << requestFigures << std::endl;
  sharedMemory->signalServer();
  sharedMemory->waitTenedor();
  // Leer la respuesta del servidor
  std::string responseFigures = sharedMemory->read();
  this->requestColabHTTPAdapter.setServerFiguresByResponse(responseFigures);
  std::cout << "Respuesta recibida del servidor: " << responseFigures << std::endl;
  // Activar cliente
  sharedMemory->signalClient();

  while (true) {
    sharedMemory->waitTenedor();  // Esperar mensaje del cliente

    if (sharedMemory->getFinished()) {  // Verificar si debe terminar
      break;
    }

    // Leer el mensaje del cliente
    std::string message = sharedMemory->read();
    std::cout << "Mensaje recibido del cliente: " << message << std::endl;

    // Procesar el mensaje y enviarlo al servidor
    std::string responseS;
    int result = requestColabHTTPAdapter.buildRequest(message, responseS);

    char *response = (char *)responseS.c_str(); //NOLINT
    if (result == EXIT_FAILURE) {
      // response = (char *)"Error 404 Not found"; //NOLINT
      sharedMemory->write(response);
      std::cout << "Respuesta enviada al cliente: " << response << std::endl;
      sharedMemory->signalClient();
    } else {
      sharedMemory->write(response);
      std::cout << "Mensaje enviado al servidor: " << responseS << std::endl;
      sharedMemory->signalServer();
      sharedMemory->waitTenedor();
      // Leer la respuesta del servidor
      responseS = sharedMemory->read();
      std::cout << "Respuesta recibida del servidor: " << responseS << std::endl;
      std::string responseH = this->requestColabHTTPAdapter.buildResponse(responseS);
      sharedMemory->write(const_cast<char *>(responseH.c_str()));
      std::cout << "Respuesta enviada al cliente: " << responseH << std::endl;
      sharedMemory->signalClient();
    }
  }
  return nullptr;
}

void *Tenedor::runTenedor(void *arg) {
  Tenedor *tenedor =
      new Tenedor();  // Convertir el argumento a un puntero a Tenedor
  void *result = tenedor->run(arg);
  delete tenedor;
  return result;  // Llamar al método de instancia
}

std::string Tenedor::validateRequest(std::string request) {

  return "error";
}

std::string Tenedor::validateResponse(std::string response) {
  if (HTTP.find(response) != HTTP.end()) {
    return HTTP[response];
  }
  return "Error 404 Not found";
}

