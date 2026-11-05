#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "persistence.h"

// Static helper to write BST nodes recursively to file
static void writeBSTNodeToFile(Book* node, FILE* fp) {
    if (node == NULL || fp == NULL) return;
    
    // Write current node in format: ID|Title|Author|DueDate|IsReturned
    fprintf(fp, "%d|%s|%s|%d|%d\n", node->id, node->title, node->author, node->dueDate, node->isReturned ? 1 : 0);
    
    writeBSTNodeToFile(node->left, fp);
    writeBSTNodeToFile(node->right, fp);
}

bool saveLibraryData(Book* bstRoot, const char* filename) {
    if (!filename) return false;
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s' for writing.\n", filename);
        return false;
    }
    
    writeBSTNodeToFile(bstRoot, fp);
    
    fclose(fp);
    return true;
}

int loadLibraryData(const char* filename, Book** bstRoot, MinHeap* heap) {
    if (!filename || !bstRoot || !heap) return 0;
    
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        // File might not exist on first run, which is acceptable
        return 0;
    }
    
    char line[512];
    int loadedCount = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Trim trailing newlines / carriage returns
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (len == 0) continue;
        
        // Parse line: ID|Title|Author|DueDate|IsReturned
        int id = 0;
        int dueDate = 0;
        bool isRet = false;
        char title[MAX_TITLE_LEN] = {0};
        char author[MAX_AUTHOR_LEN] = {0};
        
        // Find pipes
        char* p1 = strchr(line, '|');
        if (!p1) continue;
        *p1 = '\0';
        id = atoi(line);
        
        char* p2 = strchr(p1 + 1, '|');
        if (!p2) continue;
        *p2 = '\0';
        strncpy(title, p1 + 1, MAX_TITLE_LEN - 1);
        
        char* p3 = strchr(p2 + 1, '|');
        if (!p3) continue;
        *p3 = '\0';
        strncpy(author, p2 + 1, MAX_AUTHOR_LEN - 1);
        
        char* p4 = strchr(p3 + 1, '|');
        if (p4) {
            *p4 = '\0';
            dueDate = atoi(p3 + 1);
            isRet = (atoi(p4 + 1) != 0);
        } else {
            dueDate = atoi(p3 + 1);
        }
        
        // Create book node
        Book* newBook = createBook(id, title, author, dueDate);
        if (newBook) {
            newBook->isReturned = isRet;
            *bstRoot = insertBST(*bstRoot, newBook);
            insertMinHeap(heap, newBook);
            loadedCount++;
        }
    }
    
    fclose(fp);
    return loadedCount;
}
