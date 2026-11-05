#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bst.h"

int main(void) {
    printf("--- [MODULE 1 TEST: BST Core & Catalog Logic] ---\n");
    
    Book* root = NULL;
    
    // Create books
    Book* b1 = createBook(101, "The Great Gatsby", "F. Scott Fitzgerald", 20261015);
    Book* b2 = createBook(102, "1984", "George Orwell", 20260920);
    Book* b3 = createBook(103, "To Kill a Mockingbird", "Harper Lee", 20260925);
    Book* b4 = createBook(104, "Brave New World", "Aldous Huxley", 20261101);
    Book* b5 = createBook(105, "Animal Farm", "George Orwell", 20260915);
    
    assert(b1 != NULL);
    assert(b2 != NULL);
    assert(b3 != NULL);
    assert(b4 != NULL);
    assert(b5 != NULL);
    
    // Insert into BST
    root = insertBST(root, b1);
    root = insertBST(root, b2);
    root = insertBST(root, b3);
    root = insertBST(root, b4);
    root = insertBST(root, b5);
    
    // Check tree size
    int size = getBSTSize(root);
    printf("Total books in BST: %d (Expected: 5)\n", size);
    assert(size == 5);
    
    // Search lookup tests
    Book* found1 = searchBST(root, "1984");
    assert(found1 != NULL);
    assert(found1->id == 102);
    assert(strcmp(found1->author, "George Orwell") == 0);
    printf("Found '1984' -> ID: %d, Author: %s\n", found1->id, found1->author);
    
    Book* found2 = searchBST(root, "the great gatsby"); // case-insensitive search check
    assert(found2 != NULL);
    assert(found2->id == 101);
    printf("Found 'the great gatsby' (case-insensitive) -> ID: %d\n", found2->id);
    
    Book* notFound = searchBST(root, "Nonexistent Book");
    assert(notFound == NULL);
    printf("Searching 'Nonexistent Book' correctly returned NULL.\n");
    
    // Substring fuzzy search test
    Book* subResults[5];
    int subCount = 0;
    searchSubstringBST(root, "great", subResults, &subCount, 5);
    assert(subCount == 1);
    assert(subResults[0]->id == 101);
    printf("Substring search for 'great' matched 1 book: '%s'\n", subResults[0]->title);
    
    // In-order traversal test (alphabetical sorting verification)
    Book* sortedList[5];
    int index = 0;
    inorderBSTToArray(root, sortedList, &index);
    assert(index == 5);
    
    printf("\nAlphabetical Order Check:\n");
    for (int i = 0; i < 5; i++) {
        printf("  %d. %s (Author: %s, Due: %d)\n", i + 1, sortedList[i]->title, sortedList[i]->author, sortedList[i]->dueDate);
        if (i > 0) {
            assert(strcmp(sortedList[i - 1]->title, sortedList[i]->title) <= 0);
        }
    }
    
    // Clean up
    freeBST(root);
    printf("\n[MODULE 1 PASSED]: All BST tests succeeded cleanly!\n\n");
    return 0;
}
