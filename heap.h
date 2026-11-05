#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include "bst.h"

typedef struct MinHeap {
    Book** books;
    int size;
    int capacity;
} MinHeap;

/**
 * Initializes a new MinHeap with specified initial capacity.
 * Returns NULL on allocation failure.
 */
MinHeap* initMinHeap(int capacity);

/**
 * Frees the MinHeap container array.
 * Note: Does NOT free the underlying Book pointers (owned by BST).
 */
void freeMinHeap(MinHeap* heap);

/**
 * Swaps two Book pointers.
 */
void swapBooks(Book** a, Book** b);

/**
 * Restores the min-heap property upward from a given index.
 */
void bubbleUp(MinHeap* heap, int index);

/**
 * Restores the min-heap property downward from a given index.
 */
void minHeapify(MinHeap* heap, int index);

/**
 * Inserts a Book pointer into the MinHeap. Dynamically resizes if full.
 */
void insertMinHeap(MinHeap* heap, Book* book);

/**
 * Returns the Book with the earliest due date without removing it. O(1) access.
 * Returns NULL if heap is empty.
 */
Book* peekMinHeap(MinHeap* heap);

/**
 * Removes and returns the Book with the earliest due date. O(log n).
 * Returns NULL if heap is empty.
 */
Book* extractMinHeap(MinHeap* heap);

/**
 * Retrieves all books from the heap whose dueDate is strictly less than referenceDate
 * or on/before referenceDate. Fills overdueList array up to maxResults.
 * Returns the count of overdue books found.
 */
int getOverdueBooks(MinHeap* heap, int referenceDate, Book** overdueList, int maxResults);

/**
 * Removes a specific book from the heap by its pointer or ID.
 */
bool removeFromMinHeap(MinHeap* heap, Book* target);

#endif // HEAP_H
