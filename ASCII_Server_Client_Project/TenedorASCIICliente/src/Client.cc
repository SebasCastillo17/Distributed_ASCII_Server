// Copyright [2025] <Sebastian Orozco>

/**
 * @file Client main
 * @brief to test the client
 */

#include "../include/Server/Socket.h"

#include <cstdio>
#include <string>
#include <cstring>

#define SIZE_BUFFER 256

int main(int argc, char *argv[]) { 
  VSocket *s;
  char buffer[SIZE_BUFFER] = {0};
  s = new Socket('s');
  
  s->MakeConnection("127.0.0.5", 8080);
  
  if (argc > 1) {
    s->Write(argv[1]); 
  } else {
    s->Write("GET /get/Goku HTTP/1.1\r\nHost: localhost\r\n\r\n");
  }
  

  std::string response;
  int bytesRead;
  
  do {
    memset(buffer, 0, SIZE_BUFFER);
    bytesRead = s->Read(buffer, SIZE_BUFFER);
    
    if (bytesRead > 0) {
      response.append(buffer, bytesRead);
      printf("Recibidos %d bytes\n", bytesRead);
    }
  } while (bytesRead > 0);
  printf("Respuesta completa:\n%s\n", response.c_str());
  s->Close();
  return 0; 
}
