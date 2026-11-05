#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdbool.h>
#include <stdint.h>
#include "bst.h"
#include "heap.h"

#define BENCHMARK_RECORD_COUNT 1000000
#define BENCHMARK_10M_COUNT    10000000

// Flat record structure for traditional linear scan array (used in 1M dataset)
typedef struct FlatBook {
    int id;
    char title[MAX_TITLE_LEN];
    char author[MAX_AUTHOR_LEN];
    int dueDate;
    bool isReturned;
} FlatBook;

// 1M Benchmark Dataset Container
typedef struct BenchmarkDataset {
    Book* bstRoot;           // Dual-Engine BST Index (1M nodes)
    MinHeap* heap;           // Dual-Engine Min-Heap Index (1M pointers)
    FlatBook* flatCatalog;   // Traditional Flat Contiguous Array (1M structs)
    int count;               // Total record count (1,000,000)
    bool isInitialized;      // Flag indicating allocation status
    double memoryUsageMB;    // Computed total memory footprint in MB
} BenchmarkDataset;

// 10M Slab Allocation Dataset Container (Contiguous Pool for Zero-Fragmentation & Zero-Stack Overhead)
typedef struct Benchmark10MDataset {
    Book* pool;              // Single contiguous memory slab (10,000,000 Book structs)
    Book** heap;             // Min-Heap pointer array (10,000,000 Book* references)
    Book* bstRoot;           // Root pointer to iterative BST
    int count;               // Total record count (10,000,000)
    bool isInitialized;      // Status flag
    double memoryUsageMB;    // Footprint in MB (~2920 MB)
} Benchmark10MDataset;

// Single Benchmark Comparison Result
typedef struct BenchmarkResult {
    char queryTitle[MAX_TITLE_LEN];
    int foundId;
    bool dualFound;
    bool traditionalFound;
    
    // Title Search Latencies
    double bstTimeUs;          // BST Search time in microseconds (us)
    double linearSearchTimeMs; // Traditional Linear Search time in milliseconds (ms)
    double searchSpeedup;      // Speedup multiplier (Linear / BST)

    // Due Date Peek Latencies
    double minHeapTimeUs;      // Min-Heap Peek time in microseconds (us)
    double linearHeapTimeMs;   // Traditional Min Due Search time in milliseconds (ms)
    double heapSpeedup;        // Speedup multiplier (Linear / Heap)
    
    int dualMinDueDate;
    int traditionalMinDueDate;
    
    bool correctnessPass;      // True if results of both approaches match perfectly
} BenchmarkResult;

// Automated Stress Loop Aggregated Results
typedef struct StressTestResult {
    int totalIterations;
    double avgBstTimeUs;
    double avgLinearSearchMs;
    double avgSearchSpeedup;
    
    double avgMinHeapTimeUs;
    double avgLinearHeapMs;
    double avgHeapSpeedup;
    
    int totalPassedCorrectness;
} StressTestResult;

/**
 * Initializes the 1,000,000 synthetic record dataset dynamically on the heap.
 */
BenchmarkDataset* initBenchmarkDataset(int recordCount);

/**
 * Destroys the 1M dataset, releasing memory.
 */
void freeBenchmarkDataset(BenchmarkDataset* dataset);

/**
 * Runs side-by-side comparison for a given query title (1M dataset).
 */
BenchmarkResult runDualVsTraditionalBenchmark(BenchmarkDataset* dataset, const char* queryTitle);

/**
 * Runs automated 10-iteration continuous stress loop (1M dataset).
 */
StressTestResult runStressLoopBenchmark(BenchmarkDataset* dataset, int iterations);

/**
 * Initializes the 10,000,000 synthetic record dataset using a SINGLE CONTIGUOUS SLAB ALLOCATION.
 * Uses non-recursive iterative BST insertion to eliminate stack overflow risk.
 */
Benchmark10MDataset* initBenchmark10MDataset(int recordCount);

/**
 * Destroys the 10M slab dataset with O(1) instant memory teardown (free pool & heap).
 */
void freeBenchmark10MDataset(Benchmark10MDataset* dataset);

/**
 * Runs side-by-side comparison for a given query title across 10,000,000 records.
 */
BenchmarkResult run10MBenchmark(Benchmark10MDataset* dataset, const char* queryTitle);

/**
 * Runs automated continuous stress loop across 10,000,000 records.
 */
StressTestResult run10MStressLoopBenchmark(Benchmark10MDataset* dataset, int iterations);

/**
 * High-precision cross-platform monotonic timer returning seconds.
 */
double getHighResTimeInSeconds(void);

#endif // BENCHMARK_H
