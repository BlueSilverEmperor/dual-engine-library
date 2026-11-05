#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bst.h"
#include "heap.h"

int main(void) {
    printf("--- [MODULE 2 TEST: Min-Heap Core & Overdue Engine] ---\n");
    
    // Create several books with varying due dates
    Book* b1 = createBook(1, "Book Alpha", "Author A", 20261015);
    Book* b2 = createBook(2, "Book Beta", "Author B", 20260910); // Earliest due date
    Book* b3 = createBook(3, "Book Gamma", "Author C", 20260920);
    Book* b4 = createBook(4, "Book Delta", "Author D", 20260905); // Even earlier!
    Book* b5 = createBook(5, "Book Epsilon", "Author E", 20261225);
    
    MinHeap* heap = initMinHeap(4); // Test initial capacity and dynamic expansion
    assert(heap != NULL);
    assert(heap->capacity == 4);
    
    // Insert books into Min-Heap
    insertMinHeap(heap, b1);
    insertMinHeap(heap, b2);
    insertMinHeap(heap, b3);
    insertMinHeap(heap, b4);
    insertMinHeap(heap, b5); // Capacity expands to 8
    
    assert(heap->size == 5);
    assert(heap->capacity >= 5);
    
    // Peek root (O(1) access check)
    Book* rootBook = peekMinHeap(heap);
    assert(rootBook != NULL);
    printf("Min-Heap Root Due Date: %d (Expected: 20260905 - Book Delta)\n", rootBook->dueDate);
    assert(rootBook->dueDate == 20260905);
    assert(strcmp(rootBook->title, "Book Delta") == 0);
    
    // Overdue evaluation test
    // Reference date: 20260920 (Sept 20, 2026)
    Book* overdueList[10];
    int count = getOverdueBooks(heap, 20260920, overdueList, 10);
    printf("\nOverdue Evaluation against reference date 20260920:\n");
    printf("  Found %d overdue/due books:\n", count);
    for (int i = 0; i < count; i++) {
        printf("    - ID %d: '%s' (Due: %d)\n", overdueList[i]->id, overdueList[i]->title, overdueList[i]->dueDate);
    }
    assert(count == 3); // 20260905, 20260910, 20260920
    assert(overdueList[0]->dueDate == 20260905);
    assert(overdueList[1]->dueDate == 20260910);
    assert(overdueList[2]->dueDate == 20260920);
    
    // Extract Min extraction order test (Priority Queue check)
    printf("\nExtracting all books in ascending order of due date:\n");
    int previousDate = 0;
    while (heap->size > 0) {
        Book* minB = extractMinHeap(heap);
        printf("  Extracted: '%s' -> Due Date: %d\n", minB->title, minB->dueDate);
        assert(minB->dueDate >= previousDate);
        previousDate = minB->dueDate;
    }
    assert(heap->size == 0);
    assert(peekMinHeap(heap) == NULL);
    
    // Memory cleanup
    freeMinHeap(heap);
    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b5);
    
    printf("\n[MODULE 2 PASSED]: All Min-Heap tests succeeded cleanly!\n\n");
    return 0;
}
