#ifndef BST_H
#define BST_H

#define MAX_TITLE_LEN 128
#define MAX_AUTHOR_LEN 128

typedef struct Book {
    int id;
    char title[MAX_TITLE_LEN];
    char author[MAX_AUTHOR_LEN];
    int dueDate; // Date formatted as integer YYYYMMDD (e.g., 20260930)
    bool isReturned; // false = Checked Out/Active, true = Returned/Available
    struct Book* left;
    struct Book* right;
} Book;

/**
 * Creates a new Book node with allocated memory.
 * Returns NULL if memory allocation fails.
 */
Book* createBook(int id, const char* title, const char* author, int dueDate);

/**
 * Inserts a Book node into the BST, sorted alphabetically by title.
 * If title is identical, falls back to sorting by ID.
 * Returns the root of the tree.
 */
Book* insertBST(Book* root, Book* newBook);

/**
 * Iteratively inserts a Book node into the BST without recursion. O(1) stack memory.
 */
Book* insertBSTIterative(Book* root, Book* newBook);

/**
 * Searches the BST for a book with exact or case-insensitive matching title.
 * Returns pointer to Book if found, NULL otherwise.
 */
Book* searchBST(Book* root, const char* title);

/**
 * Iteratively searches the BST for a book with matching title. O(1) stack memory.
 */
Book* searchBSTIterative(Book* root, const char* title);

/**
 * Searches the BST for a book with a specific ID.
 */
Book* searchBSTById(Book* root, int id);

/**
 * Recursively searches the BST for books whose titles contain the query substring (case-insensitive).
 * Fills results array up to maxResults and updates resultCount.
 */
void searchSubstringBST(Book* root, const char* query, Book* results[], int* resultCount, int maxResults);

/**
 * Frees all memory occupied by the BST nodes recursively.
 */
void freeBST(Book* root);

/**
 * Returns total count of books in the BST.
 */
int getBSTSize(Book* root);

/**
 * Performs in-order traversal to populate an array of Book pointers.
 */
void inorderBSTToArray(Book* root, Book** array, int* index);

#endif // BST_H
