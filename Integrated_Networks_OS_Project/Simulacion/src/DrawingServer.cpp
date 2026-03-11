// Copyright [2025] <AIGroup>

#include <server/DrawingServer.h>
#include <common/SharedMemory.h>

#include <iomanip>
#include <stdexcept>


void *DrawingServer::run(void *data) {
  SharedMemory *sharedMemory = static_cast<SharedMemory *>(data);

  while (true) {
    sharedMemory->waitServer();  // Esperar mensaje del tenedor

    if (sharedMemory->getFinished()) {  // Verificar si debe terminar
      break;
    }

    // Leer el mensaje del tenedor
    std::string message = sharedMemory->read();
    std::cout << "Mensaje recibido del tenedor de servidor: " << message << std::endl;

    message = this->GCP.buildAnswer(message);
    char *response = (char*)message.c_str();

    std::cout << "Respuesta del servidor al tenedor: " << message << std::endl;
    
    sharedMemory->write(response);  // Mensaje para el Fork
    sharedMemory->signalTenedor();
  }
  return nullptr;
}

void *DrawingServer::runDrawingServer(void *arg) {
  DrawingServer *drawingServer = new DrawingServer();
  void *result = drawingServer->run(arg);
  delete drawingServer;
  return result;
}
