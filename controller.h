#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include "bst.h"
#include "heap.h"

typedef struct LibraryController {
    Book* bstRoot;
    MinHeap* heap;
    char dataFilePath[256];
    int nextAutoId;
} LibraryController;

/**
 * Initializes the LibraryController, allocating internal structures and loading persistence data.
 */
LibraryController* initController(const char* dataFilePath);

/**
 * Destroys the LibraryController, freeing BST nodes, MinHeap array, and controller struct.
 */
void freeController(LibraryController* ctrl);

/**
 * Adds a new book to both the BST catalog and the Min-Heap overdue engine.
 * If id is <= 0, automatically assigns nextAutoId.
 */
Book* addBookController(LibraryController* ctrl, int id, const char* title, const char* author, int dueDate);

/**
 * Searches catalog by title using BST search. O(log n).
 */
Book* searchBookController(LibraryController* ctrl, const char* title);

/**
 * Searches catalog by title substring (case-insensitive fuzzy search).
 * Populates outputBuffer up to maxCount and returns the count of matching books found.
 */
int searchBooksSubstringController(LibraryController* ctrl, const char* query, Book** outputBuffer, int maxCount);

/**
 * Evaluates overdue status against a reference date.
 * Populates outputBuffer and returns number of overdue books found.
 */
int checkOverdueController(LibraryController* ctrl, int referenceDate, Book** outputBuffer, int maxCount);

/**
 * Retrieves all books in in-order alphabetical order.
 * Populates outputBuffer and returns total count.
 */
int getCatalogController(LibraryController* ctrl, Book** outputBuffer, int maxCount);

/**
 * Saves current library state to disk.
 */
bool saveController(LibraryController* ctrl);

/**
 * Reloads library state from disk.
 */
bool loadController(LibraryController* ctrl);

/**
 * Returns total number of books in catalog.
 */
int getTotalBooksController(LibraryController* ctrl);

/**
 * Marks a book as returned given its ID.
 * Sets isReturned = true and saves state to disk.
 * Returns true if book found and marked, false otherwise.
 */
bool returnBookController(LibraryController* ctrl, int id);

#endif // CONTROLLER_H
