// Copyright [2025] <Sebastian Orozco>

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

// define 256 for the moment, it can be more
#define COUNTBLOCKS 255

// define the size of blocks, it is permanent
#define SIZEBLOCK 256

// Define the name of the disk
#define DISKNAME "disk.dat"

#include <stdint.h>
#include <stdio.h>
#include "Structures.h"

/// @brief Mount the filesystem
/// @return EXIT_SUCCESS if the filesystem is mounted, EXIT_FAILURE otherwise
uint8_t mountSystem();

/// @brief Write a file in the filesystem
/// @param name name of the file
/// @param path path of the file
/// @return EXIT_SUCCESS if the file is written, EXIT_FAILURE otherwise
uint8_t writeFile(char *name, char *path);

/// @brief Read a file in the filesystem and show the content
/// @param path of the file
/// @return EXIT_SUCCESS if the file is read, EXIT_FAILURE otherwise
uint8_t showContent(char *path);

char *getSystemInfo();

uint8_t existASCII(char *name);

/**
 * @brief 
 *
 * @param name 
 * @return 
 */
char *extractBlock(char *name, size_t number);

#ifdef __cplusplus
}
#endif

#endif  // FILESYSTEM_H

