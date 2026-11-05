#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "benchmark.h"

// High-precision cross-platform monotonic timer returning seconds
double getHighResTimeInSeconds(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

// Case-insensitive string comparison helper
static int strcasecmp_custom(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2) {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) return diff;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// ============================================================================
// 1-MILLION RECORD BENCHMARK MODULE
// ============================================================================

BenchmarkDataset* initBenchmarkDataset(int recordCount) {
    if (recordCount <= 0) recordCount = BENCHMARK_RECORD_COUNT;

    BenchmarkDataset* ds = (BenchmarkDataset*)malloc(sizeof(BenchmarkDataset));
    if (!ds) {
        fprintf(stderr, "Error: Failed to allocate BenchmarkDataset structure\n");
        return NULL;
    }

    ds->count = recordCount;
    ds->bstRoot = NULL;
    ds->heap = initMinHeap(recordCount);
    ds->flatCatalog = (FlatBook*)malloc(sizeof(FlatBook) * recordCount);

    if (!ds->heap || !ds->flatCatalog) {
        fprintf(stderr, "Error: Failed to allocate 1M record arrays\n");
        if (ds->heap) freeMinHeap(ds->heap);
        if (ds->flatCatalog) free(ds->flatCatalog);
        free(ds);
        return NULL;
    }

    static const char* TOPICS[] = {
        "Algorithms", "Clean Architecture", "Systems Programming", "Data Structures",
        "Operating Systems", "Quantum Computing", "Compiler Design", "Database Internals",
        "Computer Networks", "Distributed Systems", "Machine Learning", "Artificial Intelligence",
        "Cryptography", "Parallel Computing", "Embedded Systems", "Computer Vision"
    };
    int numTopics = sizeof(TOPICS) / sizeof(TOPICS[0]);

    int* perm = (int*)malloc(sizeof(int) * recordCount);
    if (!perm) {
        fprintf(stderr, "Error: Failed to allocate permutation array\n");
        free(ds->flatCatalog);
        freeMinHeap(ds->heap);
        free(ds);
        return NULL;
    }

    for (int i = 0; i < recordCount; i++) {
        perm[i] = i;

        int id = i + 1;
        char titleBuf[MAX_TITLE_LEN];
        char authorBuf[MAX_AUTHOR_LEN];

        if (i == 100000) {
            snprintf(titleBuf, sizeof(titleBuf), "Clean Code Architecture");
            snprintf(authorBuf, sizeof(authorBuf), "Robert C. Martin");
        } else if (i == 500000) {
            snprintf(titleBuf, sizeof(titleBuf), "Data Structures & Algorithms");
            snprintf(authorBuf, sizeof(authorBuf), "Donald Knuth");
        } else if (i == 900000) {
            snprintf(titleBuf, sizeof(titleBuf), "Systems Programming in C");
            snprintf(authorBuf, sizeof(authorBuf), "Dennis Ritchie");
        } else {
            snprintf(titleBuf, sizeof(titleBuf), "%s Guide Vol %d", TOPICS[i % numTopics], (i / numTopics) + 1);
            snprintf(authorBuf, sizeof(authorBuf), "Author_%d", (i % 5000) + 1);
        }

        int day = (i % 28) + 1;
        int month = ((i / 28) % 4) + 9;
        int dueDate = 20260000 + month * 100 + day;

        ds->flatCatalog[i].id = id;
        strncpy(ds->flatCatalog[i].title, titleBuf, MAX_TITLE_LEN - 1);
        ds->flatCatalog[i].title[MAX_TITLE_LEN - 1] = '\0';
        strncpy(ds->flatCatalog[i].author, authorBuf, MAX_AUTHOR_LEN - 1);
        ds->flatCatalog[i].author[MAX_AUTHOR_LEN - 1] = '\0';
        ds->flatCatalog[i].dueDate = dueDate;
        ds->flatCatalog[i].isReturned = false;
    }

    srand(42);
    for (int i = recordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }

    for (int idx = 0; idx < recordCount; idx++) {
        int i = perm[idx];
        Book* newBook = createBook(
            ds->flatCatalog[i].id,
            ds->flatCatalog[i].title,
            ds->flatCatalog[i].author,
            ds->flatCatalog[i].dueDate
        );

        if (newBook) {
            ds->bstRoot = insertBSTIterative(ds->bstRoot, newBook);
            insertMinHeap(ds->heap, newBook);
        }
    }

    free(perm);

    double bstMemMB = (double)recordCount * sizeof(Book) / (1024.0 * 1024.0);
    double heapMemMB = (double)recordCount * sizeof(Book*) / (1024.0 * 1024.0);
    double flatMemMB = (double)recordCount * sizeof(FlatBook) / (1024.0 * 1024.0);
    ds->memoryUsageMB = bstMemMB + heapMemMB + flatMemMB;
    ds->isInitialized = true;

    return ds;
}

void freeBenchmarkDataset(BenchmarkDataset* dataset) {
    if (!dataset) return;

    if (dataset->bstRoot) {
        freeBST(dataset->bstRoot);
        dataset->bstRoot = NULL;
    }

    if (dataset->heap) {
        freeMinHeap(dataset->heap);
        dataset->heap = NULL;
    }

    if (dataset->flatCatalog) {
        free(dataset->flatCatalog);
        dataset->flatCatalog = NULL;
    }

    free(dataset);
}

BenchmarkResult runDualVsTraditionalBenchmark(BenchmarkDataset* dataset, const char* queryTitle) {
    BenchmarkResult res;
    memset(&res, 0, sizeof(res));

    if (!dataset || !dataset->isInitialized || !queryTitle || queryTitle[0] == '\0') {
        return res;
    }

    strncpy(res.queryTitle, queryTitle, sizeof(res.queryTitle) - 1);

    double startBst = getHighResTimeInSeconds();
    Book* bstMatch = searchBSTIterative(dataset->bstRoot, queryTitle);
    double endBst = getHighResTimeInSeconds();
    res.bstTimeUs = (endBst - startBst) * 1000000.0;

    if (bstMatch) {
        res.dualFound = true;
        res.foundId = bstMatch->id;
    }

    double startLinear = getHighResTimeInSeconds();
    FlatBook* linearMatch = NULL;
    for (int i = 0; i < dataset->count; i++) {
        if (strcasecmp_custom(dataset->flatCatalog[i].title, queryTitle) == 0) {
            linearMatch = &dataset->flatCatalog[i];
            break;
        }
    }
    double endLinear = getHighResTimeInSeconds();
    res.linearSearchTimeMs = (endLinear - startLinear) * 1000.0;

    if (linearMatch) {
        res.traditionalFound = true;
        if (!res.dualFound) res.foundId = linearMatch->id;
    }

    double linearSearchUs = res.linearSearchTimeMs * 1000.0;
    double bstUsGuard = (res.bstTimeUs > 0.001) ? res.bstTimeUs : 0.001;
    res.searchSpeedup = linearSearchUs / bstUsGuard;

    double startHeap = getHighResTimeInSeconds();
    Book* heapRoot = peekMinHeap(dataset->heap);
    double endHeap = getHighResTimeInSeconds();
    res.minHeapTimeUs = (endHeap - startHeap) * 1000000.0;
    res.dualMinDueDate = heapRoot ? heapRoot->dueDate : 0;

    double startHeapLinear = getHighResTimeInSeconds();
    int minDue = 99999999;
    for (int i = 0; i < dataset->count; i++) {
        if (!dataset->flatCatalog[i].isReturned && dataset->flatCatalog[i].dueDate < minDue) {
            minDue = dataset->flatCatalog[i].dueDate;
        }
    }
    double endHeapLinear = getHighResTimeInSeconds();
    res.linearHeapTimeMs = (endHeapLinear - startHeapLinear) * 1000.0;
    res.traditionalMinDueDate = (minDue < 99999999) ? minDue : 0;

    double linearHeapUs = res.linearHeapTimeMs * 1000.0;
    double heapUsGuard = (res.minHeapTimeUs > 0.001) ? res.minHeapTimeUs : 0.001;
    res.heapSpeedup = linearHeapUs / heapUsGuard;

    bool searchCorrect = (res.dualFound == res.traditionalFound) &&
                         (!res.dualFound || res.foundId == linearMatch->id);
    bool heapCorrect = (res.dualMinDueDate == res.traditionalMinDueDate);
    res.correctnessPass = searchCorrect && heapCorrect;

    return res;
}

StressTestResult runStressLoopBenchmark(BenchmarkDataset* dataset, int iterations) {
    StressTestResult stress;
    memset(&stress, 0, sizeof(stress));

    if (!dataset || !dataset->isInitialized) return stress;
    if (iterations <= 0) iterations = 10;

    stress.totalIterations = iterations;

    const char* sampleQueries[] = {
        "Clean Code Architecture",
        "Data Structures & Algorithms",
        "Systems Programming in C",
        "Algorithms Guide Vol 1",
        "Operating Systems Guide Vol 10",
        "Quantum Computing Guide Vol 5",
        "NonExistentBookKey_XYZ999",
        "Compiler Design Guide Vol 2",
        "Database Internals Guide Vol 7",
        "Artificial Intelligence Guide Vol 3"
    };
    int queryCount = sizeof(sampleQueries) / sizeof(sampleQueries[0]);

    double totalBstUs = 0.0;
    double totalLinearMs = 0.0;
    double totalHeapUs = 0.0;
    double totalLinearHeapMs = 0.0;

    for (int i = 0; i < iterations; i++) {
        const char* q = sampleQueries[i % queryCount];
        BenchmarkResult r = runDualVsTraditionalBenchmark(dataset, q);

        totalBstUs += r.bstTimeUs;
        totalLinearMs += r.linearSearchTimeMs;
        totalHeapUs += r.minHeapTimeUs;
        totalLinearHeapMs += r.linearHeapTimeMs;

        if (r.correctnessPass) {
            stress.totalPassedCorrectness++;
        }
    }

    stress.avgBstTimeUs = totalBstUs / iterations;
    stress.avgLinearSearchMs = totalLinearMs / iterations;
    double avgLinearSearchUs = stress.avgLinearSearchMs * 1000.0;
    stress.avgSearchSpeedup = avgLinearSearchUs / (stress.avgBstTimeUs > 0.001 ? stress.avgBstTimeUs : 0.001);

    stress.avgMinHeapTimeUs = totalHeapUs / iterations;
    stress.avgLinearHeapMs = totalLinearHeapMs / iterations;
    double avgLinearHeapUs = stress.avgLinearHeapMs * 1000.0;
    stress.avgHeapSpeedup = avgLinearHeapUs / (stress.avgMinHeapTimeUs > 0.001 ? stress.avgMinHeapTimeUs : 0.001);

    return stress;
}

// ============================================================================
// 10-MILLION (10M) RECORD SLAB BENCHMARK MODULE
// ============================================================================

// Helper min-heapify for 10M pointer array
static void minHeapify10M(Book** heap, int size, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size && heap[left]->dueDate < heap[smallest]->dueDate) {
        smallest = left;
    }
    if (right < size && heap[right]->dueDate < heap[smallest]->dueDate) {
        smallest = right;
    }

    if (smallest != index) {
        Book* tmp = heap[index];
        heap[index] = heap[smallest];
        heap[smallest] = tmp;
        minHeapify10M(heap, size, smallest);
    }
}

// Divide-and-conquer balanced tree construction from sorted array slab (Stack depth <= 24 frames)
static Book* buildBalancedBSTFromSortedArray(Book* pool, int start, int end) {
    if (start > end) return NULL;
    int mid = start + (end - start) / 2;
    Book* root = &pool[mid];
    root->left = buildBalancedBSTFromSortedArray(pool, start, mid - 1);
    root->right = buildBalancedBSTFromSortedArray(pool, mid + 1, end);
    return root;
}

static int compareBooksByTitle(const void* a, const void* b) {
    const Book* b1 = (const Book*)a;
    const Book* b2 = (const Book*)b;
    int cmp = strcasecmp_custom(b1->title, b2->title);
    if (cmp != 0) return cmp;
    return b1->id - b2->id;
}

Benchmark10MDataset* initBenchmark10MDataset(int recordCount) {
    if (recordCount <= 0) recordCount = BENCHMARK_10M_COUNT;

    Benchmark10MDataset* ds = (Benchmark10MDataset*)malloc(sizeof(Benchmark10MDataset));
    if (!ds) {
        fprintf(stderr, "Error: Failed to allocate Benchmark10MDataset structure\n");
        return NULL;
    }

    ds->count = recordCount;
    ds->bstRoot = NULL;

    // Single Contiguous Slab Allocation for 10,000,000 Book Structs
    ds->pool = (Book*)malloc(sizeof(Book) * recordCount);
    // Min-Heap Array for 10,000,000 Book Pointers
    ds->heap = (Book**)malloc(sizeof(Book*) * recordCount);

    if (!ds->pool || !ds->heap) {
        fprintf(stderr, "Error: Failed to allocate 10M record contiguous memory slab (RAM space insufficient)\n");
        if (ds->pool) free(ds->pool);
        if (ds->heap) free(ds->heap);
        free(ds);
        return NULL;
    }

    static const char* TOPICS[] = {
        "Algorithms", "Clean Architecture", "Systems Programming", "Data Structures",
        "Operating Systems", "Quantum Computing", "Compiler Design", "Database Internals",
        "Computer Networks", "Distributed Systems", "Machine Learning", "Artificial Intelligence",
        "Cryptography", "Parallel Computing", "Embedded Systems", "Computer Vision"
    };
    int numTopics = sizeof(TOPICS) / sizeof(TOPICS[0]);

    // Step 1: Populate 10,000,000 records in contiguous slab
    for (int i = 0; i < recordCount; i++) {
        ds->pool[i].id = i + 1;

        if (i == 1000000) {
            snprintf(ds->pool[i].title, sizeof(ds->pool[i].title), "Clean Code Architecture 10M");
            snprintf(ds->pool[i].author, sizeof(ds->pool[i].author), "Robert C. Martin");
        } else if (i == 5000000) {
            snprintf(ds->pool[i].title, sizeof(ds->pool[i].title), "Data Structures & Algorithms 10M");
            snprintf(ds->pool[i].author, sizeof(ds->pool[i].author), "Donald Knuth");
        } else if (i == 9000000) {
            snprintf(ds->pool[i].title, sizeof(ds->pool[i].title), "Systems Programming in C 10M");
            snprintf(ds->pool[i].author, sizeof(ds->pool[i].author), "Dennis Ritchie");
        } else {
            snprintf(ds->pool[i].title, sizeof(ds->pool[i].title), "%s Guide Vol %d", TOPICS[i % numTopics], (i / numTopics) + 1);
            snprintf(ds->pool[i].author, sizeof(ds->pool[i].author), "Author_%d", (i % 10000) + 1);
        }

        int day = (i % 28) + 1;
        int month = ((i / 28) % 4) + 9;
        ds->pool[i].dueDate = 20260000 + month * 100 + day;
        ds->pool[i].isReturned = false;
        ds->pool[i].left = NULL;
        ds->pool[i].right = NULL;

        ds->heap[i] = &ds->pool[i];
    }

    // Step 2: Sort 10,000,000 records in slab by title for ultra-fast balanced tree construction
    qsort(ds->pool, recordCount, sizeof(Book), compareBooksByTitle);

    // Step 3: Build perfectly balanced BST in O(N) time (tree depth exactly 24 levels, max stack <= 24 frames)
    ds->bstRoot = buildBalancedBSTFromSortedArray(ds->pool, 0, recordCount - 1);

    // Step 4: Re-assign heap pointers to pool items & perform Fast O(N) Bottom-Up Min-Heapification
    for (int i = 0; i < recordCount; i++) {
        ds->heap[i] = &ds->pool[i];
    }
    for (int i = (recordCount / 2) - 1; i >= 0; i--) {
        minHeapify10M(ds->heap, recordCount, i);
    }

    double poolMemMB = (double)recordCount * sizeof(Book) / (1024.0 * 1024.0);
    double heapMemMB = (double)recordCount * sizeof(Book*) / (1024.0 * 1024.0);
    ds->memoryUsageMB = poolMemMB + heapMemMB;
    ds->isInitialized = true;

    return ds;
}

void freeBenchmark10MDataset(Benchmark10MDataset* dataset) {
    if (!dataset) return;

    // Instant O(1) Memory Release: free single slab and heap pointer array
    if (dataset->pool) {
        free(dataset->pool);
        dataset->pool = NULL;
    }
    if (dataset->heap) {
        free(dataset->heap);
        dataset->heap = NULL;
    }
    dataset->bstRoot = NULL;
    free(dataset);
}

BenchmarkResult run10MBenchmark(Benchmark10MDataset* dataset, const char* queryTitle) {
    BenchmarkResult res;
    memset(&res, 0, sizeof(res));

    if (!dataset || !dataset->isInitialized || !queryTitle || queryTitle[0] == '\0') {
        return res;
    }

    strncpy(res.queryTitle, queryTitle, sizeof(res.queryTitle) - 1);

    // -------------------------------------------------------------
    // TEST 1A: Iterative O(log n) BST Title Search across 10,000,000 Records
    // -------------------------------------------------------------
    double startBst = getHighResTimeInSeconds();
    Book* bstMatch = searchBSTIterative(dataset->bstRoot, queryTitle);
    double endBst = getHighResTimeInSeconds();
    res.bstTimeUs = (endBst - startBst) * 1000000.0; // Microseconds

    if (bstMatch) {
        res.dualFound = true;
        res.foundId = bstMatch->id;
    }

    // -------------------------------------------------------------
    // TEST 1B: Traditional Sequential O(n) Contiguous Slab Search
    // -------------------------------------------------------------
    double startLinear = getHighResTimeInSeconds();
    Book* linearMatch = NULL;
    for (int i = 0; i < dataset->count; i++) {
        if (strcasecmp_custom(dataset->pool[i].title, queryTitle) == 0) {
            linearMatch = &dataset->pool[i];
            break;
        }
    }
    double endLinear = getHighResTimeInSeconds();
    res.linearSearchTimeMs = (endLinear - startLinear) * 1000.0; // Milliseconds

    if (linearMatch) {
        res.traditionalFound = true;
        if (!res.dualFound) res.foundId = linearMatch->id;
    }

    double linearSearchUs = res.linearSearchTimeMs * 1000.0;
    double bstUsGuard = (res.bstTimeUs > 0.001) ? res.bstTimeUs : 0.001;
    res.searchSpeedup = linearSearchUs / bstUsGuard;

    // -------------------------------------------------------------
    // TEST 2A: O(1) Min-Heap Root Peek across 10,000,000 Records
    // -------------------------------------------------------------
    double startHeap = getHighResTimeInSeconds();
    Book* heapRoot = (dataset->heap && dataset->count > 0) ? dataset->heap[0] : NULL;
    double endHeap = getHighResTimeInSeconds();
    res.minHeapTimeUs = (endHeap - startHeap) * 1000000.0; // Microseconds
    res.dualMinDueDate = heapRoot ? heapRoot->dueDate : 0;

    // -------------------------------------------------------------
    // TEST 2B: Traditional Sequential O(n) Minimum Due Date Scan
    // -------------------------------------------------------------
    double startHeapLinear = getHighResTimeInSeconds();
    int minDue = 99999999;
    for (int i = 0; i < dataset->count; i++) {
        if (!dataset->pool[i].isReturned && dataset->pool[i].dueDate < minDue) {
            minDue = dataset->pool[i].dueDate;
        }
    }
    double endHeapLinear = getHighResTimeInSeconds();
    res.linearHeapTimeMs = (endHeapLinear - startHeapLinear) * 1000.0; // Milliseconds
    res.traditionalMinDueDate = (minDue < 99999999) ? minDue : 0;

    double linearHeapUs = res.linearHeapTimeMs * 1000.0;
    double heapUsGuard = (res.minHeapTimeUs > 0.001) ? res.minHeapTimeUs : 0.001;
    res.heapSpeedup = linearHeapUs / heapUsGuard;

    bool searchCorrect = (res.dualFound == res.traditionalFound) &&
                         (!res.dualFound || res.foundId == linearMatch->id);
    bool heapCorrect = (res.dualMinDueDate == res.traditionalMinDueDate);
    res.correctnessPass = searchCorrect && heapCorrect;

    return res;
}

StressTestResult run10MStressLoopBenchmark(Benchmark10MDataset* dataset, int iterations) {
    StressTestResult stress;
    memset(&stress, 0, sizeof(stress));

    if (!dataset || !dataset->isInitialized) return stress;
    if (iterations <= 0) iterations = 10;

    stress.totalIterations = iterations;

    const char* sampleQueries[] = {
        "Clean Code Architecture 10M",
        "Data Structures & Algorithms 10M",
        "Systems Programming in C 10M",
        "Algorithms Guide Vol 1",
        "Operating Systems Guide Vol 10",
        "Quantum Computing Guide Vol 500",
        "NonExistent10MRecord_XYZ999",
        "Compiler Design Guide Vol 25",
        "Database Internals Guide Vol 70",
        "Artificial Intelligence Guide Vol 30"
    };
    int queryCount = sizeof(sampleQueries) / sizeof(sampleQueries[0]);

    double totalBstUs = 0.0;
    double totalLinearMs = 0.0;
    double totalHeapUs = 0.0;
    double totalLinearHeapMs = 0.0;

    for (int i = 0; i < iterations; i++) {
        const char* q = sampleQueries[i % queryCount];
        BenchmarkResult r = run10MBenchmark(dataset, q);

        totalBstUs += r.bstTimeUs;
        totalLinearMs += r.linearSearchTimeMs;
        totalHeapUs += r.minHeapTimeUs;
        totalLinearHeapMs += r.linearHeapTimeMs;

        if (r.correctnessPass) {
            stress.totalPassedCorrectness++;
        }
    }

    stress.avgBstTimeUs = totalBstUs / iterations;
    stress.avgLinearSearchMs = totalLinearMs / iterations;
    double avgLinearSearchUs = stress.avgLinearSearchMs * 1000.0;
    stress.avgSearchSpeedup = avgLinearSearchUs / (stress.avgBstTimeUs > 0.001 ? stress.avgBstTimeUs : 0.001);

    stress.avgMinHeapTimeUs = totalHeapUs / iterations;
    stress.avgLinearHeapMs = totalLinearHeapMs / iterations;
    double avgLinearHeapUs = stress.avgLinearHeapMs * 1000.0;
    stress.avgHeapSpeedup = avgLinearHeapUs / (stress.avgMinHeapTimeUs > 0.001 ? stress.avgMinHeapTimeUs : 0.001);

    return stress;
}
