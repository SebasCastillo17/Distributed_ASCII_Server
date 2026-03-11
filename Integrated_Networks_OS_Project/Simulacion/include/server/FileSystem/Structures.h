// Copyright [2025] <Sebastian Orozco>

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define DATAPACKET 255

// TO-DO(SOC): Anyone can access to these structs?

/// @struct MemBlock
/// @brief Struct to manage the content of the block
struct MemBlock;

/// @struct DirBlock
/// @brief Struct to manage the directory
struct DirBlock;

/// @struct ASCIIBlock
/// @brief Struct to manage the ASCII block
/// @var ASCIIBlock::info Pointer to the content of the block
typedef struct ASCIIBlock {
  struct MemBlock *info;
} ASCIIBlock_t;

/// @struct Directory
/// @brief Struct to manage the directory
/// @var Directory::nameFile Pointer to the name of the file
typedef struct Directory {
  char *nameFile;
  uint8_t firstBlock;
} Directory_t;

/// @brief Extract the content of the file .txt
/// @param block Pointer to the ASCIIBlock_t
/// @param pathFile Path of the file .txt
/// @param readNumber Number of reads
/// @return EXIST_SUCCESS if is correctly,
/// EXIT_FAILURE if the file don't exists, 2 if the file is finished and correct
uint8_t extractContent(ASCIIBlock_t *block, char *pathFile, size_t readNumber);

// Create objects

/// @brief Create a new ASCIIBlock_t
/// @param block Pointer to the ASCIIBlock_t
void createBlock(ASCIIBlock_t *block);

// Getters

/// @brief Get the content of the block
/// @param block Pointer to the ASCIIBlock_t
/// @return Pointer to the content of the block
char *getData(ASCIIBlock_t *block);

/// @brief Get the next block of the file
/// @param block Pointer to the ASCIIBlock_t
/// @return The next block of the file
uint8_t getNextBlock(ASCIIBlock_t *block);

// Setters

/// @brief Set the next block of the file
/// @param block Pointer to the ASCIIBlock_t
/// @param next The next block of the file
void setNextBlock(ASCIIBlock_t *block, uint8_t next);

/// @brief Set the content of the block
/// @param block Pointer to the ASCIIBlock_t
/// @param Data Pointer to the content of the block
void setData(ASCIIBlock_t *block, char *Data);
// Free

/**
 * @brief Liberate the memory
 *
 * @param block struct ASCIIBlock
 */
void freeASCIIBlock(ASCIIBlock_t *block);

#endif  // STRUCTURES_H

