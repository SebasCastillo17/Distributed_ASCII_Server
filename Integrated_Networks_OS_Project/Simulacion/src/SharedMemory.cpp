// Copyright [2025] <AIGroup>

#include "common/SharedMemory.h"

SharedMemory::SharedMemory() : writeIndex(0), readIndex(0), finished(false) {
  sem_init(&clientSemaphore, 0, 0);
  sem_init(&tenedorSemaphore, 0, 0);
  sem_init(&serverSemaphore, 0, 0);
  sem_init(&finishMutex, 0,
           1);  // Inicializar el mutex para la variable `finished`
}

SharedMemory::~SharedMemory() {
  sem_destroy(&clientSemaphore);
  sem_destroy(&tenedorSemaphore);
  sem_destroy(&serverSemaphore);
  sem_destroy(&finishMutex);
}

void SharedMemory::write(char *value) {
  int i = 0;
  while (value[i] != '\0') {  // Copiar cada carácter de la cadena
    buffer[writeIndex] = value[i];
    writeIndex = (writeIndex + 1) %
                 1024;  // Avanzar el índice de escritura de forma circular
    i++;
  }
  buffer[writeIndex] = '\0';  // Agregar un carácter nulo al final de la cadena
  writeIndex =
      (writeIndex + 1) % 1024;  // Avanzar el índice para el carácter nulo
}

std::string SharedMemory::read() {
  std::string result;
  while (buffer[readIndex] != '\0') {  // Leer hasta encontrar el carácter nulo
    result += buffer[readIndex];
    readIndex = (readIndex + 1) %
                1024;  // Avanzar el índice de lectura de forma circular
  }
  readIndex =
      (readIndex + 1) % 1024;  // Avanzar el índice para saltar el carácter nulo
  return result;
}

void SharedMemory::signalClient() { sem_post(&clientSemaphore); }

void SharedMemory::waitClient() { sem_wait(&clientSemaphore); }

void SharedMemory::signalTenedor() { sem_post(&tenedorSemaphore); }

void SharedMemory::waitTenedor() { sem_wait(&tenedorSemaphore); }

void SharedMemory::signalServer() { sem_post(&serverSemaphore); }

void SharedMemory::waitServer() { sem_wait(&serverSemaphore); }

void SharedMemory::setFinished(bool value) { this->finished = value; }

bool SharedMemory::getFinished() {
  bool value = this->finished;
  return value;
}
