// Copyright [2025] <Sebastian Orozco>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileSystem.h"
#include "Structures.h"

// TO-DO(SOC): Manages errors?

/// @brief Allocate a block in the directory
/// @param name Name of the file
/// @param d Directory_t
/// @return EXIT_SUCCESS if the block is allocated, EXIT_FAILURE otherwise
uint8_t allocateInDirectory(char *name, Directory_t *d);

/// @brief Allocate a block in the content
/// @param firstBlock First block of the file
/// @param path Path of the file
/// @return EXIT_SUCCESS if the block is allocated, EXIT_FAILURE otherwise
uint8_t allocateContent(uint8_t firstBlock, char *path);

/// @brief Show the content of the file
/// @return EXIT_SUCCESS if the file exists, EXIT_FAILURE otherwise
uint8_t checkBlockAvaible();

/// @brief Check if the filesystem is mounted
/// @return EXIT_SUCCESS if the filesystem is mounted, EXIT_FAILURE otherwise
uint8_t checkMounted();

/// @brief Validate if the file exists
/// @param file Directory_t
/// @param name Name of the file
/// @return EXIT_SUCCESS if the file exists, EXIT_FAILURE otherwise
/// @note The filename must be less than 16 characters
uint8_t existFile(Directory_t *file, char *name);

uint8_t mountSystem() {
  if (checkMounted() == EXIT_SUCCESS) {
    return EXIT_SUCCESS;
  }
  uint8_t *directory = calloc(sizeof(uint8_t), SIZEBLOCK);
  FILE *disk = fopen(DISKNAME, "wb");
  directory[255] = 1;
  fwrite(directory, sizeof(directory[0]), SIZEBLOCK, disk);
  fseek(disk, 255, SEEK_SET);
  fclose(disk);
  free(directory);
  return EXIT_SUCCESS;
}

uint8_t writeFile(char *name, char *path) {
  if (checkMounted() == EXIT_FAILURE) {
    printf("Mount the disk first\n");
    return EXIT_FAILURE;
  }
  if (strlen(name) > 16) {
    return EXIT_FAILURE;
  }
  Directory_t d;
  if (!existFile(&d, name)) {
    printf("Change name, file already exist on this disk\n");
    free(d.nameFile);
    return EXIT_FAILURE;
  }
  d.nameFile = (char *)calloc(sizeof(char), 16); // NOLINT
  uint8_t error = allocateInDirectory(name, &d);
  if (error == EXIT_FAILURE) {
    printf("Insuficient space on directory\n");
    return error;
  }
  free(d.nameFile);
  error = allocateContent(d.firstBlock, path);
  if (error == EXIT_FAILURE) {
    printf("Insuficient space on this disk");
    return error;
  }
  printf("The file %s it's done\n", name);
  return EXIT_SUCCESS;
}

uint8_t allocateInDirectory(char *name, Directory_t *d) {
  size_t error = 0;
  strcpy(d->nameFile, name); // NOLINT
  d->firstBlock = 1;
  FILE *file = fopen(DISKNAME, "rb+");
  if (!file) {
    printf("Can't open the disk\n");
    return EXIT_FAILURE;
  }
  for (size_t i = 0; i < SIZEBLOCK && d->firstBlock != 0; i += 17) {
    error = fseek(file, i + 16, SEEK_SET);
    fread(&(d->firstBlock), sizeof(uint8_t), 1, file);
    if (error) {
      fclose(file);
      return EXIT_FAILURE;
    }
  }
  error = fseek(file, -1, SEEK_CUR);
  if (error) {
    fclose(file);
    return EXIT_FAILURE;
  }
  d->firstBlock = checkBlockAvaible();
  printf("first Block %i\n", d->firstBlock);
  fwrite(&(d->firstBlock), sizeof(uint8_t), 1, file);
  fseek(file, -17, SEEK_CUR);
  fwrite((d->nameFile), sizeof(char), 16, file);
  fclose(file);
  return EXIT_SUCCESS;
}

uint8_t allocateContent(uint8_t firstBlock, char *path) {
  ASCIIBlock_t block;
  createBlock(&block);
  uint8_t extracted = extractContent(&block, path, 0);
  if (extracted == EXIT_FAILURE) {
    printf("The path file is incorrect\n");
    return extracted;
  }
  FILE *file = fopen(DISKNAME, "rb+");
  fseek(file, firstBlock * SIZEBLOCK, SEEK_SET);
  char *data = getData(&block);
  fwrite(data, sizeof(data[0]), 255, file);
  if (extracted == 2) {
    uint8_t finish = 255;
    fwrite(&finish, sizeof(finish), 1, file);
    freeASCIIBlock(&block);
    fclose(file);
    return EXIT_SUCCESS;
  }
  setNextBlock(&block, checkBlockAvaible());
  uint8_t nextBlock = getNextBlock(&block);
  fwrite(&nextBlock, sizeof(uint8_t), 1, file);
  size_t index = 1;
  while (index < COUNTBLOCKS) {
    fseek(file, nextBlock * SIZEBLOCK, SEEK_SET);
    extracted = extractContent(&block, path, index);
    data = getData(&block);
    fwrite(data, sizeof(data[0]), 255, file);
    if (extracted == 2) {
      uint8_t finish = 255;
      fwrite(&finish, sizeof(finish), 1, file);
      freeASCIIBlock(&block);
      fclose(file);
      return EXIT_SUCCESS;
    }
    setNextBlock(&block, checkBlockAvaible());
    nextBlock = getNextBlock(&block);
    fwrite(&nextBlock, sizeof(uint8_t), 1, file);
    index++;
  }
  // TO-DO(SOC): manage better this in the future
  freeASCIIBlock(&block);
  fclose(file);
  printf("Can't allocate the block, insuficient space");
  return EXIT_FAILURE;
}

uint8_t showContent(char *name) {
  if (!name) {
    printf("Name??\n");
    return EXIT_FAILURE;
  }
  Directory_t d;
  // validate if file exists
  if (existFile(&d, name)) {
    printf("Can't find that file\n");
    return EXIT_FAILURE;
  }
  ASCIIBlock_t block;
  createBlock(&block);
  // 255 indicates the finish of the file
  FILE *disk = fopen(DISKNAME, "rb");
  fseek(disk, d.firstBlock * SIZEBLOCK, SEEK_SET);
  char *data = getData(&block);
  fread(data, sizeof(getData(&block)[0]), 255, disk);
  uint8_t nextBlock = 0;
  fread(&nextBlock, sizeof(uint8_t), 1, disk);
  setNextBlock(&block, nextBlock);
  printf("%s", data);
  while (getNextBlock(&block) != 255) {
    fseek(disk, nextBlock * SIZEBLOCK, SEEK_SET);
    fread(data, sizeof(data[0]), 255, disk);
    nextBlock = getNextBlock(&block);
    fread(&nextBlock, sizeof(uint8_t), 1, disk);
    setNextBlock(&block, nextBlock);
    printf("%s", data);
  }
  freeASCIIBlock(&block);
  printf("\n");
  fclose(disk);
  return EXIT_SUCCESS;
}

uint8_t existFile(Directory_t *file, char *name) {
  FILE *disk = fopen(DISKNAME, "rb");
  char *NameFile = calloc(sizeof(char), 16);
  size_t i = 0;
  while (strcmp(name, NameFile) && i < SIZEBLOCK) {
    fread(NameFile, sizeof(NameFile[0]), 16, disk);
    fseek(disk, 1, SEEK_CUR);
    i += 17;
  }
  if (i >= 255) {
    fclose(disk);
    free(NameFile);
    return EXIT_FAILURE;
  }
  file->nameFile = NameFile;
  fseek(disk, -1, SEEK_CUR);
  fread(&(file->firstBlock), sizeof(uint8_t), 1, disk);
  fclose(disk);
  return EXIT_SUCCESS;
}

uint8_t existASCII(char *name) {
  FILE *disk = fopen(DISKNAME, "rb");
  char *NameFile = calloc(sizeof(char), 16);
  size_t i = 0;
  while (strcmp(name, NameFile) && i < SIZEBLOCK) {
    fread(NameFile, sizeof(NameFile[0]), 16, disk);
    fseek(disk, 1, SEEK_CUR);
    i += 17;
  }
  if (i >= 255) {
    fclose(disk);
    free(NameFile);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

uint8_t checkBlockAvaible() {
  FILE *disk = fopen(DISKNAME, "rb+");
  // locate the first block
  uint8_t block = 0;
  // If content is removed this 'for' can find the block and return it
  for (size_t i = 1; i < COUNTBLOCKS; i++) {
    if (fseek(disk, i * 256, SEEK_SET)) {
      break;
    }

    if (fseek(disk, 255, SEEK_CUR)) {
      break;
    }

    if (fread(&block, sizeof(uint8_t), 1, disk) != 1) {
      break;
    }
    if (block == 0) {
      fseek(disk, -1, SEEK_CUR);
      uint8_t restrictB = 255;
      fwrite(&restrictB, sizeof(uint8_t), 1, disk);
      fclose(disk);
      return i;
    }
  }

  fseek(disk, 255, SEEK_SET);
  fread(&block, sizeof(uint8_t), 1, disk);
  if (block != 255) {
    uint8_t next = 1;
    next += block;
    fseek(disk, 255, SEEK_SET);
    fwrite(&next, sizeof(next), 1, disk);
    fclose(disk);
    return block;
  }
  fclose(disk);
  return EXIT_FAILURE;
}

char *extractBlock(char *name, size_t number) {
  if (!name) {
    printf("Name??\n");
    return NULL;
  }
  Directory_t d;
  // validate if file exists
  if (number == 0) {
    if (existFile(&d, name)) {
      printf("Can't find that file\n");
      return NULL;
    }
    free(d.nameFile);
    ASCIIBlock_t block;
    createBlock(&block);
    FILE *disk = fopen(DISKNAME, "rb");
    fseek(disk, d.firstBlock * SIZEBLOCK, SEEK_SET);
    char *data = getData(&block);
    fread(data, sizeof(getData(&block)[0]), 256, disk);
    fclose(disk);
    return data;
  }
  ASCIIBlock_t block;
  createBlock(&block);
  FILE *disk = fopen(DISKNAME, "rb");
  fseek(disk, SIZEBLOCK * number, SEEK_SET);
  char *data = getData(&block);
  fread(data, sizeof(char), 256, disk);
  fclose(disk);
  return data;
}

uint8_t checkMounted() {
  FILE *verifyDisk = fopen(DISKNAME, "r");
  if (!verifyDisk) {
    return EXIT_FAILURE;
  }
  fclose(verifyDisk);
  return EXIT_SUCCESS;
}

