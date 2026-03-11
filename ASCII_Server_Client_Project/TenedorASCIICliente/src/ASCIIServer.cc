// Copyright [2025] <Sebastian Orozco>

#include "../include/FileSystem/FileSystem.h"
#include "../include/Server/ASCIIServer.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  ASCIIServer server;
  mountSystem();
  writeFile((char *)"Personal.txt", (char *)"ascii/Personal.txt");
  writeFile((char *)"Goku.txt", (char *)"ascii/Goku.txt");
  writeFile((char *)"Gogeta.txt", (char *)"ascii/Gogeta.txt");
  writeFile((char *)"Foca.txt", (char *)"ascii/Foca.txt");
  writeFile((char *)"CocaCola.txt", (char *)"ascii/CocaCola.txt");
  writeFile((char *)"Exitante.txt", (char *)"ascii/Exitante.txt");
  writeFile((char *)"GoyoCat.txt", (char *)"ascii/GoyoCat.txt");
  if(argc > 1) {
    printf("IPV6\n");    
    server.init(8090, 10, 's', true);
    return 0;
  }
  getSystemInfo();
  server.init(8090, 10);
  return 0;
}

