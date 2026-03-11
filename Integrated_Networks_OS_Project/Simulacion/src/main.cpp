// Copyright [2025] <AIGroup>

#include <client/Client.h>
#include <server/DrawingServer.h>
#include <common/SharedMemory.h>
#include <fork/Tenedor.h>
#include <pthread.h>

#include <iomanip>
#include <iostream>

int main() {
  Client client;
  DrawingServer drawingServer;
  Tenedor tenedor;

  // Crear una instancia de SharedMemory
  SharedMemory sharedMemory;

  // Crear hilos y pasar la instancia de SharedMemory como argumento
  pthread_create(&(client.OwnThread), nullptr, Client::runClient,
                 &sharedMemory);
  pthread_create(&(tenedor.OwnThread), nullptr, Tenedor::runTenedor,
                 &sharedMemory);
  pthread_create(&(drawingServer.OwnThread), nullptr,
                 DrawingServer::runDrawingServer, &sharedMemory);

  // Esperar a que los hilos terminen
  pthread_join(client.OwnThread, nullptr);
  pthread_join(tenedor.OwnThread, nullptr);
  pthread_join(drawingServer.OwnThread, nullptr);

  std::cout << "Programa terminado." << std::endl;
  return 0;
}
