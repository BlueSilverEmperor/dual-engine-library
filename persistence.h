#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>
#include "bst.h"
#include "heap.h"

/**
 * Saves all records from the BST into a text file.
 * Returns true on success, false on failure.
 */
bool saveLibraryData(Book* bstRoot, const char* filename);

/**
 * Loads records from text file and populates BOTH bstRoot and minHeap simultaneously.
 * Returns the number of books successfully loaded.
 */
int loadLibraryData(const char* filename, Book** bstRoot, MinHeap* heap);

#endif // PERSISTENCE_H
