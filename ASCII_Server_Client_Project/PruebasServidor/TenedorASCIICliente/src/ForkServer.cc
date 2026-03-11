// Copyright [2025] <Sebastian Orozco>

/**
 * @file This file is just to test in a Client Server
 * @brief Servidor Tenedor
 */

#include "../include/Server/forkServer.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  forkServer forkS;
  if(argc > 1) {
    printf("IPV6\n");   
    forkS.init(8080, 8090, 's', true);
    return 0;
  }
  forkS.init(8080, 8090);
  return 0;
}

