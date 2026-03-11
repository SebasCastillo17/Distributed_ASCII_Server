// Copyright [2025] <AIGroup>

#pragma once

#include <semaphore.h>

#include <string>
#include <iostream>

class SharedMemory {
 private:
  char buffer[1024];  // Buffer fijo de 1024 caracteres
  size_t writeIndex;  // Índice para escribir en el buffer
  size_t readIndex;  // Índice para leer del buffer

  // Semáforos para sincronización entre cliente, tenedor y servidor
  sem_t clientSemaphore;
  sem_t tenedorSemaphore;
  sem_t serverSemaphore;

  bool finished;     // Variable para indicar si el programa debe terminar
  sem_t finishMutex;  // Mutex para proteger el acceso a la variable `finished`

 public:
  SharedMemory();
  ~SharedMemory();

  // Métodos para escribir y leer del buffer
  void write(char *value);
  std::string read();

  // Métodos para manejar los semáforos
  void signalClient();
  void waitClient();
  void signalTenedor();
  void waitTenedor();
  void signalServer();
  void waitServer();

  // Métodos para manejar la condición de finalización
  void setFinished(bool value);
  bool getFinished();
};
