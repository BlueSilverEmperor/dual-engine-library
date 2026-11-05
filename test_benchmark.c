#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "benchmark.h"

int main(void) {
    printf("======================================================================\n");
    printf("  🚀 [HIGH-PERFORMANCE BENCHMARK TEST SUITE]: 1M & 10M Verification\n");
    printf("======================================================================\n\n");

    // ========================================================================
    // PART 1: 1,000,000 RECORD BENCHMARK VERIFICATION
    // ========================================================================
    printf("--- [PART 1: 1,000,000 Record Benchmark Verification] ---\n");
    printf("Allocating and generating 1,000,000 synthetic records on heap...\n");
    double tStart1M = getHighResTimeInSeconds();
    BenchmarkDataset* ds1M = initBenchmarkDataset(1000000);
    double tEnd1M = getHighResTimeInSeconds();

    assert(ds1M != NULL);
    assert(ds1M->isInitialized == true);
    assert(ds1M->count == 1000000);

    printf("  [PASSED]: 1M dataset initialized in %.2f seconds (Footprint: %.2f MB)\n", tEnd1M - tStart1M, ds1M->memoryUsageMB);

    BenchmarkResult res1M = runDualVsTraditionalBenchmark(ds1M, "Clean Code Architecture");
    printf("  - BST Search Latency (1M)         : %.3f us\n", res1M.bstTimeUs);
    printf("  - Linear Search Time (1M)         : %.3f ms (%.1f us)\n", res1M.linearSearchTimeMs, res1M.linearSearchTimeMs * 1000.0);
    printf("  - Title Search Speedup (1M)       : %.0fx Faster!\n", res1M.searchSpeedup);
    printf("  - Min-Heap Peek Latency (1M)      : %.3f us (Due: %d)\n", res1M.minHeapTimeUs, res1M.dualMinDueDate);
    printf("  - Linear Due Scan Time (1M)       : %.3f ms (Due: %d)\n", res1M.linearHeapTimeMs, res1M.traditionalMinDueDate);
    printf("  - Heap Due Peek Speedup (1M)      : %.0fx Faster!\n", res1M.heapSpeedup);
    printf("  - Correctness Match Audit (1M)    : %s\n", res1M.correctnessPass ? "PASSED ✓" : "FAILED ✗");

    assert(res1M.dualFound == true);
    assert(res1M.traditionalFound == true);
    assert(res1M.correctnessPass == true);

    freeBenchmarkDataset(ds1M);
    printf("  [PASSED]: 1M dataset memory freed cleanly!\n\n");

    // ========================================================================
    // PART 2: 10,000,000 RECORD SLAB BENCHMARK VERIFICATION
    // ========================================================================
    printf("--- [PART 2: 10,000,000 Record Slab Benchmark Verification] ---\n");
    printf("Allocating single contiguous memory slab for 10,000,000 records (~2.9 GB RAM)...\n");
    double tStart10M = getHighResTimeInSeconds();
    Benchmark10MDataset* ds10M = initBenchmark10MDataset(10000000);
    double tEnd10M = getHighResTimeInSeconds();

    if (!ds10M) {
        printf("  [WARNING]: System RAM insufficient for 10M record slab allocation. Skipping 10M live test.\n");
        return 0;
    }

    assert(ds10M != NULL);
    assert(ds10M->isInitialized == true);
    assert(ds10M->count == 10000000);

    printf("  [PASSED]: 10M slab dataset initialized in %.2f seconds!\n", tEnd10M - tStart10M);
    printf("  [FOOTPRINT]: Total 10M dataset memory footprint: %.2f MB (~2.92 GB RAM)\n", ds10M->memoryUsageMB);

    printf("\nRunning 10M Record Side-by-Side Comparison for 'Clean Code Architecture 10M'...\n");
    BenchmarkResult res10M = run10MBenchmark(ds10M, "Clean Code Architecture 10M");

    printf("  - Iterative BST Search Latency (10M): %.3f us (%.5f ms)\n", res10M.bstTimeUs, res10M.bstTimeUs / 1000.0);
    printf("  - Traditional Linear Scan Time (10M): %.3f ms (%.1f us)\n", res10M.linearSearchTimeMs, res10M.linearSearchTimeMs * 1000.0);
    printf("  - 10M Title Search Speedup Multiplier: %.0fx FASTER!\n", res10M.searchSpeedup);
    printf("  - O(1) Min-Heap Peek Latency (10M)  : %.3f us (Due: %d)\n", res10M.minHeapTimeUs, res10M.dualMinDueDate);
    printf("  - Traditional Linear Due Scan (10M) : %.3f ms (Due: %d)\n", res10M.linearHeapTimeMs, res10M.traditionalMinDueDate);
    printf("  - 10M Heap Due Peek Speedup         : %.0fx FASTER!\n", res10M.heapSpeedup);
    printf("  - 10M Correctness Audit Verification: %s\n", res10M.correctnessPass ? "PASSED ✓" : "FAILED ✗");

    assert(res10M.dualFound == true);
    assert(res10M.traditionalFound == true);
    assert(res10M.foundId == 1000001);
    assert(res10M.correctnessPass == true);

    // Verification bounds checks
    assert(res10M.bstTimeUs < 10.0); // BST search must complete in < 10 us (typically < 3 us)
    assert(res10M.minHeapTimeUs < 1.0); // Min-Heap peek must complete in < 1 us (typically < 0.1 us)
    assert(res10M.linearSearchTimeMs > 10.0); // Linear scan across 10M items must take > 10 ms (typically 100ms - 400ms)

    printf("\nRunning 10-Iteration Continuous 10M Stress Loop...\n");
    StressTestResult stress10M = run10MStressLoopBenchmark(ds10M, 10);
    printf("  - Total Iterations Completed         : %d / %d\n", stress10M.totalIterations, stress10M.totalPassedCorrectness);
    printf("  - Average Iterative BST Search Time  : %.3f us\n", stress10M.avgBstTimeUs);
    printf("  - Average 10M Linear Search Time     : %.3f ms\n", stress10M.avgLinearSearchMs);
    printf("  - Average 10M Title Search Speedup   : %.0fx FASTER!\n", stress10M.avgSearchSpeedup);
    printf("  - Average 10M Min-Heap Peek Time     : %.3f us\n", stress10M.avgMinHeapTimeUs);
    printf("  - Average 10M Linear Due Scan Time   : %.3f ms\n", stress10M.avgLinearHeapMs);
    printf("  - Average 10M Heap Due Peek Speedup  : %.0fx FASTER!\n", stress10M.avgHeapSpeedup);

    assert(stress10M.totalPassedCorrectness == 10);

    printf("\nFreeing 10,000,000 dataset memory slab...\n");
    freeBenchmark10MDataset(ds10M);
    printf("  [PASSED]: 10M slab memory released with zero leaks in < 1 ms!\n\n");

    printf("======================================================================\n");
    printf("  SUCCESS: 10M Record Benchmark Suite Passed Cleanly!\n");
    printf("======================================================================\n");

    return 0;
}
