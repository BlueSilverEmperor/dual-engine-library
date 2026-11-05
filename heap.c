#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "heap.h"

MinHeap* initMinHeap(int capacity) {
    if (capacity <= 0) capacity = 16;
    
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) {
        fprintf(stderr, "Error: Failed to allocate MinHeap structure\n");
        return NULL;
    }
    
    heap->books = (Book**)malloc(sizeof(Book*) * capacity);
    if (!heap->books) {
        fprintf(stderr, "Error: Failed to allocate MinHeap array\n");
        free(heap);
        return NULL;
    }
    
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void freeMinHeap(MinHeap* heap) {
    if (heap) {
        if (heap->books) {
            free(heap->books);
        }
        free(heap);
    }
}

void swapBooks(Book** a, Book** b) {
    Book* temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleUp(MinHeap* heap, int index) {
    if (!heap || index <= 0) return;
    
    int parent = (index - 1) / 2;
    if (heap->books[index]->dueDate < heap->books[parent]->dueDate) {
        swapBooks(&heap->books[index], &heap->books[parent]);
        bubbleUp(heap, parent);
    }
}

void minHeapify(MinHeap* heap, int index) {
    if (!heap) return;
    
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    
    if (left < heap->size && heap->books[left]->dueDate < heap->books[smallest]->dueDate) {
        smallest = left;
    }
    
    if (right < heap->size && heap->books[right]->dueDate < heap->books[smallest]->dueDate) {
        smallest = right;
    }
    
    if (smallest != index) {
        swapBooks(&heap->books[index], &heap->books[smallest]);
        minHeapify(heap, smallest);
    }
}

void insertMinHeap(MinHeap* heap, Book* book) {
    if (!heap || !book) return;
    
    // Resize array if capacity is reached
    if (heap->size >= heap->capacity) {
        int newCapacity = heap->capacity * 2;
        Book** newBooks = (Book**)realloc(heap->books, sizeof(Book*) * newCapacity);
        if (!newBooks) {
            fprintf(stderr, "Error: Reallocation failed in insertMinHeap()\n");
            return;
        }
        heap->books = newBooks;
        heap->capacity = newCapacity;
    }
    
    heap->books[heap->size] = book;
    int current = heap->size;
    heap->size++;
    
    bubbleUp(heap, current);
}

Book* peekMinHeap(MinHeap* heap) {
    if (!heap || heap->size == 0) return NULL;
    return heap->books[0];
}

Book* extractMinHeap(MinHeap* heap) {
    if (!heap || heap->size == 0) return NULL;
    
    Book* minBook = heap->books[0];
    heap->books[0] = heap->books[heap->size - 1];
    heap->size--;
    
    if (heap->size > 0) {
        minHeapify(heap, 0);
    }
    
    return minBook;
}

int getOverdueBooks(MinHeap* heap, int referenceDate, Book** overdueList, int maxResults) {
    if (!heap || !overdueList || maxResults <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < heap->size && count < maxResults; i++) {
        if (!heap->books[i]->isReturned && heap->books[i]->dueDate <= referenceDate) {
            overdueList[count++] = heap->books[i];
        }
    }
    
    // Sort overdue list by due date ascending (earliest overdue first)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (overdueList[i]->dueDate > overdueList[j]->dueDate) {
                Book* temp = overdueList[i];
                overdueList[i] = overdueList[j];
                overdueList[j] = temp;
            }
        }
    }
    
    return count;
}

bool removeFromMinHeap(MinHeap* heap, Book* target) {
    if (!heap || !target) return false;
    
    int targetIdx = -1;
    for (int i = 0; i < heap->size; i++) {
        if (heap->books[i] == target || heap->books[i]->id == target->id) {
            targetIdx = i;
            break;
        }
    }
    
    if (targetIdx == -1) return false;
    
    heap->books[targetIdx] = heap->books[heap->size - 1];
    heap->size--;
    
    if (targetIdx < heap->size) {
        bubbleUp(heap, targetIdx);
        minHeapify(heap, targetIdx);
    }
    
    return true;
}
