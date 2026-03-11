// Copyright [2025] <Sebastian Orozco>

#include "server/FileSystem/Structures.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Clean data
char cleanData[255] = {0};

struct MemBlock {
  char *data;
  uint8_t nextBlock;
};

struct DirBlock {
  char *name;
  uint8_t firstBlock;
};

uint8_t extractContent(ASCIIBlock_t *block, char *pathFile, size_t readNumber) {
  FILE *content = fopen(pathFile, "rb");
  if (!content) {
    printf("Bad patchfile\n");
    return EXIT_FAILURE;
  }
  if (fseek(content, readNumber * DATAPACKET, SEEK_SET)) {
    return EXIT_FAILURE;
  }
  char cleanBlock[255] = {0};
  memcpy(block->info->data, cleanBlock, 255);
  if (fread((block->info->data), sizeof(block->info->data[0]), 255, content) <
      255) {
    // to indicate finish
    fclose(content);
    return 2;
  }
  fclose(content);
  return EXIT_SUCCESS;
}

// Create objects

void createBlock(ASCIIBlock_t *block) {
  block->info = malloc(sizeof(struct MemBlock));
  block->info->data = calloc(sizeof(char), 255);
  block->info->nextBlock = 0;
}

// Getters

char *getData(ASCIIBlock_t *block) { return block->info->data; }
uint8_t getNextBlock(ASCIIBlock_t *block) { return block->info->nextBlock; }

// Setters

void setNextBlock(ASCIIBlock_t *block, uint8_t next) {
  block->info->nextBlock = next;
}
void setData(ASCIIBlock_t *block, char *Data) {
  memcpy(block->info->data, cleanData, 255);
  block->info->data = Data;
}

// Free
void freeASCIIBlock(ASCIIBlock_t *block) {
  free(block->info->data);
  free(block->info);
}

