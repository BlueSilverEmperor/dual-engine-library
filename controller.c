#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "controller.h"
#include "persistence.h"
#include "flow_exporter.h"

LibraryController* initController(const char* dataFilePath) {
    LibraryController* ctrl = (LibraryController*)malloc(sizeof(LibraryController));
    if (!ctrl) {
        fprintf(stderr, "Error: Failed to allocate LibraryController\n");
        return NULL;
    }
    
    ctrl->bstRoot = NULL;
    ctrl->heap = initMinHeap(16);
    ctrl->nextAutoId = 101;
    
    if (dataFilePath && dataFilePath[0] != '\0') {
        strncpy(ctrl->dataFilePath, dataFilePath, sizeof(ctrl->dataFilePath) - 1);
        ctrl->dataFilePath[sizeof(ctrl->dataFilePath) - 1] = '\0';
    } else {
        strcpy(ctrl->dataFilePath, "library_data.txt");
    }
    
    // Auto-load existing data if file exists
    loadController(ctrl);

    exportFlowSnapshot("INIT", "Controller initialized, catalog data loaded", "controller", 2, getTotalBooksController(ctrl));
    
    return ctrl;
}

void freeController(LibraryController* ctrl) {
    if (!ctrl) return;
    
    // Save data automatically on shutdown
    saveController(ctrl);

    exportFlowSnapshot("SHUTDOWN", "Application shutting down, memory freed", "persistence", 5, 0);
    
    // Free tree nodes first (owns the Book structures)
    if (ctrl->bstRoot) {
        freeBST(ctrl->bstRoot);
        ctrl->bstRoot = NULL;
    }
    
    // Free heap array structure
    if (ctrl->heap) {
        freeMinHeap(ctrl->heap);
        ctrl->heap = NULL;
    }
    
    free(ctrl);
}

Book* addBookController(LibraryController* ctrl, int id, const char* title, const char* author, int dueDate) {
    if (!ctrl || !title || title[0] == '\0' || !author || author[0] == '\0') {
        return NULL;
    }
    
    if (id <= 0) {
        id = ctrl->nextAutoId++;
    } else if (id >= ctrl->nextAutoId) {
        ctrl->nextAutoId = id + 1;
    }
    
    Book* newBook = createBook(id, title, author, dueDate);
    if (!newBook) return NULL;
    
    ctrl->bstRoot = insertBST(ctrl->bstRoot, newBook);
    insertMinHeap(ctrl->heap, newBook);
    
    saveController(ctrl);

    char details[512];
    snprintf(details, sizeof(details), "Added Book ID %d: '%s' by %s (Due: %d)", newBook->id, newBook->title, newBook->author, newBook->dueDate);
    exportFlowSnapshot("ADD_BOOK", details, "heap", 4, getTotalBooksController(ctrl));

    return newBook;
}

Book* searchBookController(LibraryController* ctrl, const char* title) {
    if (!ctrl || !title) return NULL;
    Book* res = searchBST(ctrl->bstRoot, title);

    char details[256];
    snprintf(details, sizeof(details), "Exact search for '%s': %s", title, res ? "Found" : "Not Found");
    exportFlowSnapshot("SEARCH_EXACT", details, "bst", 3, getTotalBooksController(ctrl));

    return res;
}

int searchBooksSubstringController(LibraryController* ctrl, const char* query, Book** outputBuffer, int maxCount) {
    if (!ctrl || !query || query[0] == '\0' || !outputBuffer || maxCount <= 0) return 0;
    int count = 0;
    searchSubstringBST(ctrl->bstRoot, query, outputBuffer, &count, maxCount);

    char details[256];
    snprintf(details, sizeof(details), "Substring search for '%s' returned %d match(es)", query, count);
    exportFlowSnapshot("SEARCH_SUBSTRING", details, "bst", 3, getTotalBooksController(ctrl));

    return count;
}

int checkOverdueController(LibraryController* ctrl, int referenceDate, Book** outputBuffer, int maxCount) {
    if (!ctrl || !outputBuffer || maxCount <= 0) return 0;
    int count = getOverdueBooks(ctrl->heap, referenceDate, outputBuffer, maxCount);

    char details[256];
    snprintf(details, sizeof(details), "Checked overdue items for date %d (Found %d active overdue)", referenceDate, count);
    exportFlowSnapshot("CHECK_OVERDUE", details, "heap", 4, getTotalBooksController(ctrl));

    return count;
}

int getCatalogController(LibraryController* ctrl, Book** outputBuffer, int maxCount) {
    if (!ctrl || !outputBuffer || maxCount <= 0) return 0;
    
    int total = getBSTSize(ctrl->bstRoot);
    if (total == 0) return 0;
    
    int index = 0;
    inorderBSTToArray(ctrl->bstRoot, outputBuffer, &index);

    return (index < maxCount) ? index : maxCount;
}

bool saveController(LibraryController* ctrl) {
    if (!ctrl) return false;
    bool res = saveLibraryData(ctrl->bstRoot, ctrl->dataFilePath);

    char details[512];
    snprintf(details, sizeof(details), "Saved %d catalog record(s) to '%s'", getTotalBooksController(ctrl), ctrl->dataFilePath);
    exportFlowSnapshot("PERSISTENCE_SAVE", details, "persistence", 5, getTotalBooksController(ctrl));

    return res;
}

bool loadController(LibraryController* ctrl) {
    if (!ctrl) return false;
    
    // Clear existing data before loading
    if (ctrl->bstRoot) {
        freeBST(ctrl->bstRoot);
        ctrl->bstRoot = NULL;
    }
    if (ctrl->heap) {
        freeMinHeap(ctrl->heap);
        ctrl->heap = initMinHeap(16);
    }
    
    int loaded = loadLibraryData(ctrl->dataFilePath, &ctrl->bstRoot, ctrl->heap);
    
    // Update nextAutoId based on loaded books
    int count = getBSTSize(ctrl->bstRoot);
    if (count > 0) {
        Book** allBooks = (Book**)malloc(sizeof(Book*) * count);
        if (allBooks) {
            int idx = 0;
            inorderBSTToArray(ctrl->bstRoot, allBooks, &idx);
            for (int i = 0; i < idx; i++) {
                if (allBooks[i]->id >= ctrl->nextAutoId) {
                    ctrl->nextAutoId = allBooks[i]->id + 1;
                }
            }
            free(allBooks);
        }
    }

    char details[512];
    snprintf(details, sizeof(details), "Loaded %d record(s) from disk snapshot '%s'", loaded, ctrl->dataFilePath);
    exportFlowSnapshot("PERSISTENCE_LOAD", details, "persistence", 5, getTotalBooksController(ctrl));
    
    return loaded > 0;
}

int getTotalBooksController(LibraryController* ctrl) {
    if (!ctrl) return 0;
    return getBSTSize(ctrl->bstRoot);
}

bool returnBookController(LibraryController* ctrl, int id) {
    if (!ctrl || id <= 0) return false;
    Book* book = searchBSTById(ctrl->bstRoot, id);
    if (book) {
        book->isReturned = true;
        saveController(ctrl);

        char details[256];
        snprintf(details, sizeof(details), "Marked Book ID %d ('%s') as RETURNED", book->id, book->title);
        exportFlowSnapshot("RETURN_BOOK", details, "bst", 3, getTotalBooksController(ctrl));

        return true;
    }
    return false;
}
