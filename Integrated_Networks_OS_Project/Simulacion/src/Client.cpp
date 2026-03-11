// Copyright [2025] <AIGroup>

#include <client/Client.h>
#include <common/SharedMemory.h>

#include <iomanip>

void *Client::run(void *data) {
  SharedMemory *sharedMemory = static_cast<SharedMemory *>(data);

  // Esperar al tenedor para que cargue la lista de figuras
  sharedMemory->waitClient();

  while (true) {
    // Pedir un mensaje al usuario
    std::string message;
    std::cout << "Ingrese un mensaje para enviar: ";
    std::getline(std::cin, message);  // Leer el mensaje completo
    if (message == "exit" || message == "Exit") {
      sharedMemory->setFinished(true);
      sharedMemory->signalTenedor();
      // sharedMemory->signalServer(); // No veo que se necesite
      break;
    }
    int request = validateRequest(message);
    // Convertir el mensaje a char* y enviarlo
    sharedMemory->write(
        const_cast<char *>(message.c_str()));  // Convertir std::string a char*
    std::cout << "Mensaje enviado al tenedor de cliente: " << message << std::endl;

    sharedMemory->signalTenedor();  // Notificar al tenedor
    sharedMemory->waitClient();    // Esperar respuesta del tenedor
    // Leer la respuesta del tenedor
    std::string response = sharedMemory->read();
    std::cout << "Respuesta recibida del tenedor de cliente: " << response << std::endl;
  }
  return nullptr;
}

void *Client::runClient(void *arg) {
  Client *client = new Client();   // Crear un nuevo objeto Client dinámicamente
  void *result = client->run(arg);  // Llamar al método de instancia
  delete client;                   // Liberar la memoria asignada dinámicamente
  return result;
}

int Client::validateRequest(std::string request) {
  for (int i = 0; i < 4; i++) {
    if (request == REQUEST[i]) {
      return 1;
    }
  }
  return -1;
}
