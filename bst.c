#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "bst.h"

Book* createBook(int id, const char* title, const char* author, int dueDate) {
    if (!title || !author) return NULL;
    
    Book* book = (Book*)malloc(sizeof(Book));
    if (!book) {
        fprintf(stderr, "Error: Memory allocation failed in createBook()\n");
        return NULL;
    }
    
    book->id = id;
    
    strncpy(book->title, title, MAX_TITLE_LEN - 1);
    book->title[MAX_TITLE_LEN - 1] = '\0';
    
    strncpy(book->author, author, MAX_AUTHOR_LEN - 1);
    book->author[MAX_AUTHOR_LEN - 1] = '\0';
    
    book->dueDate = dueDate;
    book->isReturned = false;
    book->left = NULL;
    book->right = NULL;
    
    return book;
}

// Case-insensitive helper function for search comparison
static int strcasecmp_custom(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = (char)tolower((unsigned char)*s1);
        char c2 = (char)tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return (unsigned char)tolower((unsigned char)*s1) - (unsigned char)tolower((unsigned char)*s2);
}

Book* insertBST(Book* root, Book* newBook) {
    if (newBook == NULL) return root;
    if (root == NULL) return newBook;
    
    int cmp = strcasecmp_custom(newBook->title, root->title);
    if (cmp < 0) {
        root->left = insertBST(root->left, newBook);
    } else if (cmp > 0) {
        root->right = insertBST(root->right, newBook);
    } else {
        // Equal titles: sort by ID to maintain deterministic binary search tree
        if (newBook->id < root->id) {
            root->left = insertBST(root->left, newBook);
        } else {
            root->right = insertBST(root->right, newBook);
        }
    }
    
    return root;
}

Book* insertBSTIterative(Book* root, Book* newBook) {
    if (newBook == NULL) return root;
    newBook->left = NULL;
    newBook->right = NULL;
    
    if (root == NULL) return newBook;
    
    Book* curr = root;
    Book* parent = NULL;
    
    while (curr != NULL) {
        parent = curr;
        int cmp = strcasecmp_custom(newBook->title, curr->title);
        if (cmp < 0) {
            curr = curr->left;
        } else if (cmp > 0) {
            curr = curr->right;
        } else {
            if (newBook->id < curr->id) {
                curr = curr->left;
            } else {
                curr = curr->right;
            }
        }
    }
    
    int cmp = strcasecmp_custom(newBook->title, parent->title);
    if (cmp < 0) {
        parent->left = newBook;
    } else if (cmp > 0) {
        parent->right = newBook;
    } else {
        if (newBook->id < parent->id) {
            parent->left = newBook;
        } else {
            parent->right = newBook;
        }
    }
    
    return root;
}

Book* searchBST(Book* root, const char* title) {
    if (root == NULL || title == NULL || title[0] == '\0') {
        return NULL;
    }
    
    int cmp = strcasecmp_custom(title, root->title);
    if (cmp == 0) {
        return root;
    } else if (cmp < 0) {
        return searchBST(root->left, title);
    } else {
        return searchBST(root->right, title);
    }
}

Book* searchBSTIterative(Book* root, const char* title) {
    if (root == NULL || title == NULL || title[0] == '\0') {
        return NULL;
    }
    
    Book* curr = root;
    while (curr != NULL) {
        int cmp = strcasecmp_custom(title, curr->title);
        if (cmp == 0) {
            return curr;
        } else if (cmp < 0) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    
    return NULL;
}

Book* searchBSTById(Book* root, int id) {
    if (root == NULL) return NULL;
    if (root->id == id) return root;
    
    Book* foundLeft = searchBSTById(root->left, id);
    if (foundLeft) return foundLeft;
    
    return searchBSTById(root->right, id);
}

static void toLowerStr(char* dest, const char* src, size_t maxLen) {
    size_t i = 0;
    while (src[i] && i < maxLen - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

void searchSubstringBST(Book* root, const char* query, Book* results[], int* resultCount, int maxResults) {
    if (root == NULL || query == NULL || query[0] == '\0' || !results || !resultCount || *resultCount >= maxResults) {
        return;
    }
    
    char titleLower[MAX_TITLE_LEN];
    char queryLower[MAX_TITLE_LEN];
    
    toLowerStr(titleLower, root->title, sizeof(titleLower));
    toLowerStr(queryLower, query, sizeof(queryLower));
    
    // In-order traversal: left subtree, current node, right subtree
    searchSubstringBST(root->left, query, results, resultCount, maxResults);
    
    if (*resultCount < maxResults && strstr(titleLower, queryLower) != NULL) {
        results[*resultCount] = root;
        (*resultCount)++;
    }
    
    searchSubstringBST(root->right, query, results, resultCount, maxResults);
}

void freeBST(Book* root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

int getBSTSize(Book* root) {
    if (root == NULL) return 0;
    return 1 + getBSTSize(root->left) + getBSTSize(root->right);
}

void inorderBSTToArray(Book* root, Book** array, int* index) {
    if (root == NULL || array == NULL || index == NULL) return;
    
    inorderBSTToArray(root->left, array, index);
    array[*index] = root;
    (*index)++;
    inorderBSTToArray(root->right, array, index);
}
