#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "controller.h"

int main(void) {
    printf("--- [MODULE 3 TEST: Persistence & Controller Bridge] ---\n");
    
    const char* testFile = "test_library_data.txt";
    
    // 1. Initialize controller
    LibraryController* ctrl = initController(testFile);
    assert(ctrl != NULL);
    assert(ctrl->bstRoot == NULL);
    assert(ctrl->heap != NULL);
    
    // 2. Add books via controller
    Book* b1 = addBookController(ctrl, 101, "Algorithms Unleashed", "Thomas Cormen", 20261010);
    Book* b2 = addBookController(ctrl, 102, "Clean Code C", "Robert Martin", 20260915);
    Book* b3 = addBookController(ctrl, 103, "Systems Programming in C", "Dennis Ritchie", 20260901);
    
    assert(b1 != NULL);
    assert(b2 != NULL);
    assert(b3 != NULL);
    assert(getTotalBooksController(ctrl) == 3);
    
    // 3. Save to file
    bool saved = saveController(ctrl);
    assert(saved == true);
    printf("Saved 3 books to disk file '%s'\n", testFile);
    
    // 4. Shut down controller (tests cleanup and auto-save)
    freeController(ctrl);
    printf("Closed controller instance.\n");
    
    // 5. Create new controller to test persistence reload across app restart
    printf("\nRestarting controller and reading back persisted file...\n");
    LibraryController* ctrl2 = initController(testFile);
    assert(ctrl2 != NULL);
    assert(getTotalBooksController(ctrl2) == 3);
    
    // 6. Verify BST search on reloaded data
    Book* searchRes = searchBookController(ctrl2, "Clean Code C");
    assert(searchRes != NULL);
    assert(searchRes->id == 102);
    assert(strcmp(searchRes->author, "Robert Martin") == 0);
    printf("Successfully searched reloaded BST for 'Clean Code C' -> ID %d, Due %d\n", searchRes->id, searchRes->dueDate);
    
    // 7. Verify Min-Heap overdue check on reloaded data
    Book* overdueList[5];
    int overdueCount = checkOverdueController(ctrl2, 20260920, overdueList, 5);
    printf("Overdue books evaluated against 20260920:\n");
    for (int i = 0; i < overdueCount; i++) {
        printf("  - '%s' (Due: %d)\n", overdueList[i]->title, overdueList[i]->dueDate);
    }
    assert(overdueCount == 2); // 20260901 and 20260915
    assert(overdueList[0]->dueDate == 20260901);
    assert(overdueList[1]->dueDate == 20260915);
    
    // 8. Test Mark as Returned
    bool returned = returnBookController(ctrl2, 102);
    assert(returned == true);
    assert(searchRes->isReturned == true);
    
    // Re-evaluate overdue count (should now be 1 since 102 was returned)
    int overdueCountAfterReturn = checkOverdueController(ctrl2, 20260920, overdueList, 5);
    assert(overdueCountAfterReturn == 1);
    printf("Successfully marked Book #102 as Returned. Overdue count reduced from 2 to 1.\n");
    
    // 9. Clean up test controller and file
    freeController(ctrl2);
    remove(testFile);
    
    printf("\n[MODULE 3 PASSED]: Persistence & Controller Bridge tests succeeded cleanly!\n\n");
    return 0;
}
