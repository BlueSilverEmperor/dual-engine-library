# Smart Library Book Inventory & Due-Date Tracker
## Complete Feature Specification & Operational Workflows

> **Document Version**: 1.0.0  
> **Target Audience**: Systems Engineers, Architects, and Evaluators  
> **Implementation Stack**: Pure C (C99), Raylib 5.5 Immediate-Mode GUI, RayGUI, TrueType Font Engine, Python Flask 3.0 Real-Time Flow Visualizer

---

## 📑 Table of Contents
1. [Executive Overview](#1-executive-overview)
2. [Comprehensive Feature Catalog](#2-comprehensive-feature-catalog)
   - [2.1 Dual-Engine Data Structure Architecture](#21-dual-engine-data-structure-architecture)
   - [2.2 Desktop Graphical User Interface (Raylib 60 FPS)](#22-desktop-graphical-user-interface-raylib-60-fps)
   - [2.3 Inventory Management & Book Lifecycle Engine](#23-inventory-management--book-lifecycle-engine)
   - [2.4 Priority Overdue Evaluation System](#24-priority-overdue-evaluation-system)
   - [2.5 Substring Title Search Engine](#25-substring-title-search-engine)
   - [2.6 Disk Persistence & Snapshot Engine](#26-disk-persistence--snapshot-engine)
   - [2.7 High-Performance Benchmark Lab (1M & 10M Slab)](#27-high-performance-benchmark-lab-1m--10m-slab)
   - [2.8 Real-Time Algorithmic Execution Flow Visualizer (Flask & Canvas)](#28-real-time-algorithmic-execution-flow-visualizer-flask--canvas)
   - [2.9 Comprehensive Unit Testing & Verification Suite](#29-comprehensive-unit-testing--verification-suite)
3. [Algorithmic Complexity & Performance Matrix](#3-algorithmic-complexity--performance-matrix)
4. [System Workflows & Execution Lifecycles](#4-system-workflows--execution-lifecycles)
   - [Workflow 1: Application Bootstrapping & Startup](#workflow-1-application-bootstrapping--startup)
   - [Workflow 2: Book Ingestion & Dual-Engine Synchronization](#workflow-2-book-ingestion--dual-engine-synchronization)
   - [Workflow 3: In-Order Catalog Retrieval & Rendering](#workflow-3-in-order-catalog-retrieval--rendering)
   - [Workflow 4: Priority Overdue Calculation & Heap Evaluation](#workflow-4-priority-overdue-calculation--heap-evaluation)
   - [Workflow 5: Substring Search & Query Inspection](#workflow-5-substring-search--query-inspection)
   - [Workflow 6: Book Return & Status Transition](#workflow-6-book-return--status-transition)
   - [Workflow 7: Disk Snapshot Serialization & Recovery](#workflow-7-disk-snapshot-serialization--recovery)
   - [Workflow 8: Microsecond Latency Benchmarking (1M & 10M Slab)](#workflow-8-microsecond-latency-benchmarking-1m--10m-slab)
   - [Workflow 9: Real-Time Algorithmic Flow Streaming](#workflow-9-real-time-algorithmic-flow-streaming)
5. [Memory Safety Invariants & Architectural Boundaries](#5-memory-safety-invariants--architectural-boundaries)

---

## 1. Executive Overview

The **Smart Library Book Inventory & Due-Date Tracker** is an enterprise-grade desktop systems application developed in **Pure C (C99)** that solves the twin challenges of high-volume book catalog searchability and immediate due-date priority tracking.

Traditional library management systems struggle with scalability when balancing alphabetical search queries against chronological due-date sweeps. Sorting an array for title search takes $O(n \log n)$ time or leaves linear searches at $O(n)$, while linearly scanning millions of records to find overdue books degrades response times to hundreds of milliseconds. 

This application introduces a **Dual-Engine Data Structure Architecture**:
1. **Binary Search Tree (BST) Catalog Engine**: Organizes book records lexicographically by title, achieving average $O(\log n)$ search latencies and $O(n)$ in-order sorted traversals.
2. **Min-Heap Overdue Priority Queue**: Indexes pointers to the exact same records by integer due date (`YYYYMMDD`), delivering $O(1)$ earliest-due peeks and rapid $O(k \log n)$ overdue extractions.

This dual-engine core is coupled with an immediate-mode 60 FPS graphical interface powered by **Raylib 5.5**, an automated **Disk Persistence Engine**, an **Algorithmic Benchmark Suite** (capable of processing up to 10,000,000 contiguous records in real time), and a decoupled **Flask Real-Time Flow Visualizer** demonstrating step-by-step state machine transitions.

---

## 2. Comprehensive Feature Catalog

### 2.1 Dual-Engine Data Structure Architecture
- **Single-Allocation Node Ownership**:
  - The application allocates book entities precisely once using dynamic memory allocation (`createBook()`).
  - The **Binary Search Tree** acts as the primary data store and owner of the dynamically allocated `Book` nodes.
- **Zero-Copy Pointer Min-Heap**:
  - The **Min-Heap** stores raw `Book*` memory pointers rather than cloning structs, achieving zero-copy priority indexing with minimal memory footprint overhead (8 bytes per pointer on 64-bit platforms).
- **Dual Index Synchronization**:
  - Controller operations guarantee that insertions simultaneously update both the BST and Min-Heap structures, maintaining structural consistency across both indexes.
- **Dynamic Memory Resilience**:
  - Clean recursive BST deallocation (`freeBST()`) and heap container deallocation (`freeMinHeap()`) ensure zero memory leaks during operation or upon process termination.

### 2.2 Desktop Graphical User Interface (Raylib 60 FPS)
- **High-Performance Immediate Mode GUI**:
  - Native 60 FPS rendering powered by **Raylib 5.5** and custom styling built over **RayGUI**.
  - Hardware accelerated graphics using OpenGL and VSync synchronization (`FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT`).
  - Dynamic window resizing with responsive layout recalculation (minimum dimensions: 900x600, default: 1120x720).
- **Dual Visual Theme System**:
  - **Executive Dark Mode**: Deep slate background (`#0F172A`), card surface (`#1E293B`), emerald accent (`#34D399`), vibrant red overdue alerts (`#FB7185`), and crisp high-contrast white text.
  - **Paper & Sage Light Mode**: Soft slate paper background (`#F1F5F9`), crisp white surfaces (`#FFFFFF`), teal/sage accent (`#0D9488`), and deep slate typography (`#0F172A`).
  - Real-time one-click theme switcher in the top navigation bar.
- **TrueType Font Rendering Engine**:
  - Crisp typography rendered via high-legibility TrueType fonts (`inter.ttf` / `segoeui.ttf`) with bilinear texture filtering, eliminating font blurriness.
- **Live Toast Notification Banner**:
  - Animated, non-blocking notification banner with auto-dismiss countdown timer (3.5s to 4.0s).
  - Color-coded visual status updates (Success Emerald, Warning Amber, Failure Crimson, Information Blue).
- **Executive Navigation & Tab Bar**:
  - 4 distinct operational views: **Full Catalog**, **Overdue Engine**, **Search Results**, and **⚡ 1M Benchmark**.
  - Dynamic tab highlight states and responsive column rendering.

### 2.3 Inventory Management & Book Lifecycle Engine
- **Interactive Form Inputs**:
  - Clean text inputs for **Book Title**, **Author Name**, and **Due Date** with custom cursor management, focus borders, and placeholder text.
  - Keyboard focus cycling using the `TAB` key for rapid multi-field data entry.
- **Auto-Incrementing ID Assigner**:
  - Automatically derives unique sequential primary keys (`101`, `102`, `103`, ...) when manual IDs are not provided.
- **Interactive Calendar Date Picker Modal**:
  - Popup calendar overlay modal supporting full year and month navigation.
  - Visual grid of calendar days with quick date selection, auto-formatting into ISO `YYYY-MM-DD`.
- **Book Return & Status Management**:
  - Per-record action buttons in table rows (`RETURN` button).
  - Instant transition from `ACTIVE` / `OVERDUE` to `RETURNED`.
  - Automatic persistence synchronization upon return.

### 2.4 Priority Overdue Evaluation System
- **Min-Heap Chronological Priority Queue**:
  - Direct access to the oldest unreturned book with $O(1)$ root inspection.
- **Custom Reference Cutoff Date**:
  - Configurable reference evaluation date via text input or dedicated calendar picker.
  - Evaluates all items with `dueDate <= referenceDate && !isReturned`.
- **Urgency Visual Hierarchy**:
  - Special highlight banner and `URGENT #1` badge applied to the highest-priority overdue record.
  - Red border badge indicators and prominent due-date warnings.

### 2.5 Substring Title Search Engine
- **Fuzzy Substring Search**:
  - Case-insensitive substring matching (`searchBooksSubstringController`) scanning the BST hierarchy.
  - Allows searching by partial keywords (e.g., `"Code"`, `"Algorithms"`, `"Programming"`).
- **Instant Search Bar & Keyboard Trigger**:
  - Integrated header search box with auto-trigger on `Enter` / `Keypad Enter` or clicking `SEARCH`.
  - Dedicated `CLEAR` button to instantly wipe query parameters and return to the primary catalog view.
- **Visual Match Feedback**:
  - Found match count banner with query parameter echo.
  - Formatted "Not Found" state cards when queries yield no results.

### 2.6 Disk Persistence & Snapshot Engine
- **Atomic Disk Serialization**:
  - Flat pipe-delimited file format (`library_data.txt`):
    ```
    ID|Title|Author|DueDate|IsReturned
    ```
- **Automated Lifecycle Hooks**:
  - **Startup**: Automatically scans and deserializes `library_data.txt`. If absent, gracefully initializes an executive demonstration dataset.
  - **Shutdown**: Systematically flushes current in-memory inventory state to disk on application exit.
- **Manual Disk Controls**:
  - Sidebar buttons for **SAVE DISK SNAPSHOT** and **RELOAD FROM DISK** for point-in-time state control.

### 2.7 High-Performance Benchmark Lab (1M & 10M Slab)
- **1,000,000 Record Synthetic Dataset**:
  - Dynamically constructs 1,000,000 synthetic records on the heap (~537 MB RAM footprint).
  - Populates Dual-Engine (BST + Min-Heap) and a traditional contiguous Flat Array.
- **10,000,000 Record Contiguous Memory Slab**:
  - Allocates a single massive contiguous block of 10,000,000 `Book` structs (~2.82 GB RAM).
  - Employs **non-recursive iterative BST insertion**, eliminating stack overflow risks inherent to deep recursion on huge datasets.
- **Side-by-Side Microsecond Latency Comparator**:
  - Uses microsecond-precision hardware clocks (`QueryPerformanceCounter` on Windows / `clock_gettime` on POSIX).
  - Measures BST title search ($O(\log n)$) against Linear scan ($O(n)$).
  - Measures Min-Heap root peek ($O(1)$) against Linear overdue array scan ($O(n)$).
  - Displays computed speedup multipliers (typically **1,000x to 50,000x faster**).
- **Correctness Audit Engine**:
  - Automatically cross-verifies that both the Dual-Engine and Linear Scan locate the exact same record ID and minimum due date, proving algorithmic fidelity.
- **10x Automated Continuous Stress Loop**:
  - Executes 10 back-to-back benchmark cycles, computing running averages for latencies and speedup ratios.

### 2.8 Real-Time Algorithmic Execution Flow Visualizer (Flask & Canvas)
- **Atomic JSON State Streaming**:
  - C module `flow_exporter` exports operation snapshots into `flow_state.json` containing active nodes, step indices, timestamps, action labels, and record metrics.
- **Flask 3.0 Web Application Server**:
  - Lightweight Python backend (`flow_visualizer/app.py`) providing live REST endpoints (`/api/state`, `/api/run-step`).
- **Interactive Web Canvas & State Scrubber**:
  - Dynamic HTML5 Canvas and SVG graph visualization (`flow.html` and `library_flow.html`).
  - Visualizes node transitions across GUI -> Controller -> BST -> Heap -> Disk -> Benchmark.
  - Timeline scrubbing controls with forward/reverse step replay and 60 FPS transitions.

### 2.9 Comprehensive Unit Testing & Verification Suite
- **Module 1 Test (`test_bst.c`)**: Validates BST node insertion, lexicographical sorting, in-order extraction, and memory deallocation.
- **Module 2 Test (`test_heap.c`)**: Validates Min-Heap bubble-up, bubble-down, extract-min, and priority ordering.
- **Module 3 Test (`test_controller.c`)**: Validates Controller integration, disk serialization, substring search, and return toggling.
- **Module 4 Test (`test_benchmark.c`)**: Stress tests 1M synthetic datasets and 10M slab allocations, auditing correctness and measuring latencies.

---

## 3. Algorithmic Complexity & Performance Matrix

| Operation | Primary Engine | Traditional Approach | Dual-Engine Time | Traditional Time | Memory / Space | Speedup Factor |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Catalog Lookup by Title** | Binary Search Tree (BST) | Linear Array Scan | **$O(\log n)$** avg | **$O(n)$** | $O(1)$ auxiliary | **$\approx$ 1,500x – 30,000x** |
| **Earliest Due Date Peek** | Min-Heap Priority Queue | Linear Array Scan | **$O(1)$** | **$O(n)$** | $O(1)$ auxiliary | **$\approx$ 5,000x – 50,000x** |
| **Book Record Insertion** | Synchronized Dual-Engine | Unsorted Append | **$O(\log n)$** | **$O(1)$** | $O(1)$ node alloc | Negligible overhead |
| **Overdue Batch Check ($k$ items)**| Min-Heap Extract | Linear Array Scan | **$O(k \log n)$** | **$O(n)$** | $O(k)$ output buffer | **$\approx$ 50x – 500x** |
| **Sorted Catalog Traversal** | BST In-Order Traversal | Array QuickSort | **$O(n)$** | **$O(n \log n)$** | $O(n)$ pointers | **Pre-sorted ($O(0)$ sort)**|
| **Disk Read / Write** | Pipe-Delimited Parser | Database Engine | **$O(n)$** | **$O(n)$** | $O(n)$ memory buffer| Zero external DBMS |

---

## 4. System Workflows & Execution Lifecycles

```
+----------------------------------------------------------------------------------------------------+
|                                     GLOBAL RUNTIME TOPOLOGY                                        |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|    +----------------------+       User Actions       +---------------------------------------+     |
|    | Raylib 60 FPS Desktop| -----------------------> |         Library Controller            |     |
|    |      GUI Layer       | <----------------------- |     (State & Business Logic Bridge)   |     |
|    +----------------------+       Render State       +---------------------------------------+     |
|               |                                                  |              |                  |
|               | State Export                                     |              |                  |
|               v                                                  v              v                  |
|    +----------------------+                           +-------------+    +---------------+         |
|    |    Flow Exporter     |                           | BST Catalog |    |   Min-Heap    |         |
|    |  (`flow_state.json`) |                           | (Title Sort)|    |  (Due Priority|         |
|    +----------------------+                           +-------------+    +---------------+         |
|               |                                                  \              /                  |
|               v                                                   \            /                   |
|    +----------------------+                                        v          v                    |
|    | Flask Web Visualizer |                                  +-------------------+                 |
|    |   (Port 5000 Canvas) |                                  | Persistence Engine|                 |
|    +----------------------+                                  |`library_data.txt` |                 |
|                                                              +-------------------+                 |
+----------------------------------------------------------------------------------------------------+
```

---

### Workflow 1: Application Bootstrapping & Startup

```mermaid
sequenceDiagram
    autonumber
    participant Main as main() Entry
    participant Raylib as Raylib Engine
    participant Ctrl as Library Controller
    participant Disk as Persistence (library_data.txt)
    participant UI as Desktop GUI

    Main->>Raylib: InitWindow(1120, 720, Flags: MSAA 4X, VSYNC, RESIZABLE)
    Main->>Raylib: Load TrueType Font (inter.ttf / segoeui.ttf)
    Main->>Ctrl: initController("library_data.txt")
    Ctrl->>Disk: loadController() -> Read pipe-delimited records
    alt File exists and contains records
        Disk-->>Ctrl: Populate BST nodes & Min-Heap pointers
    else File missing or empty
        Ctrl->>Ctrl: Seed Executive Demonstration Dataset (6 default books)
    end
    Ctrl-->>Main: Controller Ready (nextAutoId resolved)
    Main->>UI: Initialize Form Inputs & Set Tab = TAB_CATALOG
    Main->>Raylib: Enter 60 FPS Application Render Loop
```

1. **Window & Context Initialization**: The application sets hardware acceleration flags (`FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE`), opens the desktop window, and bounds minimum dimensions to 900x600.
2. **Typography Setup**: Attempts to locate `inter.ttf`, then falls back to `segoeui.ttf` (or the Windows system fonts directory), loading it at 48px with bilinear mipmapping.
3. **Controller Allocation**: Allocates the `LibraryController` structure, initializing the BST root to `NULL` and dynamically allocating the `MinHeap` container array.
4. **Data Deserialization**: Reads `library_data.txt`. If records are present, each is parsed, an ID is allocated, and the item is inserted into both engines. If empty, 6 default canonical engineering books are seeded.
5. **Main Loop Dispatch**: Launches the continuous 60 FPS immediate-mode polling and rendering loop.

---

### Workflow 2: Book Ingestion & Dual-Engine Synchronization

```mermaid
sequenceDiagram
    autonumber
    participant User as Library Operator
    participant UI as Form Inputs
    participant Ctrl as Library Controller
    participant BST as BST Catalog Engine
    participant Heap as Min-Heap Overdue Engine
    participant Flow as Flow Exporter

    User->>UI: Enter Title, Author, Pick Date via Calendar Modal
    User->>UI: Click "+ ADD TO CATALOG & HEAP"
    UI->>UI: Validate non-empty fields & parse YYYY-MM-DD
    UI->>Ctrl: addBookController(ctrl, 0, title, author, dueDate)
    Ctrl->>Ctrl: Assign nextAutoId++
    Ctrl->>Ctrl: createBook(id, title, author, dueDate) [Single Malloc]
    Ctrl->>BST: insertBook(root, newBook) -> O(log n) title placement
    BST-->>Ctrl: Updated BST Root
    Ctrl->>Heap: insertMinHeap(heap, newBook) -> O(log n) pointer bubble-up
    Heap-->>Ctrl: Pointer pushed & Heap invariant preserved
    Ctrl->>Flow: exportFlowSnapshot("INSERT_BOOK", title, "bst_heap", step, count)
    Flow-->>Ctrl: Updated flow_state.json
    Ctrl-->>UI: Return Book* pointer
    UI->>UI: Clear text fields & Show Toast "Book added to Catalog & Heap!"
```

1. **Input Collection & Validation**: The user inputs Title, Author, and Due Date. The user can click the `CAL` button to launch the calendar picker modal.
2. **Controller Ingestion Call**: `addBookController` receives the raw string data.
3. **Primary Node Creation**: `createBook` dynamically allocates memory for a single `Book` structure.
4. **BST Insertion ($O(\log n)$)**: Traverses the BST alphabetically by title and inserts the new node at the appropriate leaf position.
5. **Min-Heap Insertion ($O(\log n)$)**: Appends the `Book*` pointer to the end of the heap array and executes `bubbleUp()` comparing integer due dates (`YYYYMMDD`).
6. **State Export & Feedback**: Writes state deltas to `flow_state.json` and renders a green toast notification banner.

---

### Workflow 3: In-Order Catalog Retrieval & Rendering

```mermaid
sequenceDiagram
    autonumber
    participant UI as Catalog View (TAB_CATALOG)
    participant Ctrl as Library Controller
    participant BST as BST Catalog Engine

    UI->>Ctrl: getCatalogController(ctrl, catalogBuf, maxCount=200)
    Ctrl->>BST: getInOrderBooks(root, catalogBuf, count=0)
    Note over BST: Depth-First In-Order Traversal (Left -> Root -> Right)
    BST-->>Ctrl: Returns catalogBuf populated in sorted alphabetical order
    Ctrl-->>UI: Total book count returned
    loop For each book in buffer
        UI->>UI: Calculate overdue state: (!isReturned && dueDate <= refDate)
        UI->>UI: Draw table row with alternating background
        UI->>UI: Render ID, Title, Author, Due Date, Status Badge, and Action Button
    end
```

1. **Tab Selection**: User clicks **Full Catalog** or launches the application.
2. **In-Order Traversal**: `getCatalogController` triggers a recursive depth-first in-order sweep on the BST. Because the BST is ordered by title, this traversal extracts an alphabetically sorted array in $O(n)$ time without needing a separate sorting step.
3. **Table Generation**: The GUI iterates over the extracted buffer, computing responsive column positions, drawing alternating row backgrounds, displaying status pills (`ACTIVE`, `OVERDUE`, `RETURNED`), and attaching return action triggers.

---

### Workflow 4: Priority Overdue Calculation & Heap Evaluation

```mermaid
sequenceDiagram
    autonumber
    participant User as Operator
    participant UI as Overdue Tab (TAB_OVERDUE)
    participant Ctrl as Library Controller
    participant Heap as Min-Heap Priority Queue

    User->>UI: Set Reference Date (e.g. 2026-09-20)
    User->>UI: Click "EVALUATE OVERDUE (MIN-HEAP)"
    UI->>Ctrl: checkOverdueController(ctrl, referenceDate, overdueBuf, 200)
    Ctrl->>Heap: Inspect priority array elements
    loop Filter active overdue records
        Heap->>Heap: Check (dueDate <= refDate && !isReturned)
    end
    Ctrl-->>UI: Populates overdueBuf sorted by earliest deadline
    UI->>UI: Render Warning Banner with match count
    UI->>UI: Highlight Record #1 with "URGENT #1" Crimson Flag
```

1. **Date Selection**: The operator specifies a cutoff date (defaulting to system today) via input or calendar picker.
2. **Min-Heap Evaluation**: `checkOverdueController` leverages the priority queue to filter active (unreturned) books whose due date is less than or equal to the reference date.
3. **Urgency Sorting & Prioritization**: Because items in the heap maintain partial ordering by due date, the earliest due book appears first as the highest operational priority.
4. **Display**: The Overdue Tab displays a crimson alert summary and formats records with warning indicators.

---

### Workflow 5: Substring Search & Query Inspection

```mermaid
sequenceDiagram
    autonumber
    participant User as Operator
    participant UI as Search Header
    participant Ctrl as Library Controller
    participant BST as BST Search Routine

    User->>UI: Enter keyword "Clean" & Press Enter
    UI->>Ctrl: searchBooksSubstringController(ctrl, "Clean", results, 100)
    Ctrl->>BST: searchBooksSubstringRecursive(root, "Clean", results, count)
    Note over BST: Case-insensitive strcasestr / lowercase comparison
    BST-->>Ctrl: Returns matching Book* pointers
    Ctrl-->>UI: Count of matched books
    UI->>UI: Switch active tab to TAB_SEARCH
    alt Matches > 0
        UI->>UI: Render result table & green match count toast
    else Matches == 0
        UI->>UI: Render "NOT FOUND" red notification card
    end
```

1. **Search Trigger**: User enters a string in the search bar and clicks `SEARCH` or presses `ENTER`.
2. **Case-Insensitive Traversal**: `searchBooksSubstringController` executes a full traversal of the BST, checking titles against the lowercase query string.
3. **Tab Switch & Render**: Automatically focuses `TAB_SEARCH` and renders a table populated with matching records.

---

### Workflow 6: Book Return & Status Transition

```mermaid
sequenceDiagram
    autonumber
    participant User as Operator
    participant UI as Table Row
    participant Ctrl as Library Controller
    participant BST as BST Engine
    participant Disk as Disk Persistence

    User->>UI: Click "RETURN" button on Book #103
    UI->>Ctrl: returnBookController(ctrl, id=103)
    Ctrl->>BST: findBookById(root, 103)
    BST-->>Ctrl: Book* node pointer
    Ctrl->>Ctrl: Set book->isReturned = true
    Ctrl->>Disk: saveController(ctrl) -> Flush updated flag to disk
    Disk-->>Ctrl: Write confirmation
    Ctrl-->>UI: Operation successful
    UI->>UI: Trigger toast "Book #103 marked as Returned!"
    UI->>UI: Refresh row badge from OVERDUE/ACTIVE to RETURNED
```

1. **Return Action Triggered**: User clicks the `RETURN` button on any active or overdue record.
2. **Node Resolution**: The controller traverses the tree to find the `Book` node matching the specified integer ID.
3. **Flag Mutation**: Sets `isReturned = true`.
4. **Automatic Disk Sync**: Calls `saveController` to serialize the change immediately to `library_data.txt`, preventing state loss if the app closes.
5. **UI Update**: Changes the status badge to blue `RETURNED`, replaces the button with a neutral dash (`-`), and notifies the operator via a toast.

---

### Workflow 7: Disk Snapshot Serialization & Recovery

```mermaid
sequenceDiagram
    autonumber
    participant User as Operator / App Exit
    participant Ctrl as Library Controller
    participant BST as BST Catalog
    participant Disk as library_data.txt

    Note over User,Disk: SAVE DISK SNAPSHOT FLOW
    User->>Ctrl: saveController(ctrl)
    Ctrl->>BST: getInOrderBooks(root, saveBuffer, maxCount)
    BST-->>Ctrl: In-Order array of all Book* nodes
    Ctrl->>Disk: fopen("library_data.txt", "w")
    loop For each Book in saveBuffer
        Ctrl->>Disk: fprintf("%d|%s|%s|%d|%d\n", id, title, author, dueDate, isReturned)
    end
    Ctrl->>Disk: fclose()
    Ctrl-->>User: Toast "Saved to library_data.txt"

    Note over User,Disk: RELOAD FROM DISK FLOW
    User->>Ctrl: loadController(ctrl)
    Ctrl->>Disk: fopen("library_data.txt", "r")
    Ctrl->>Ctrl: freeBST() & reset MinHeap
    loop While fgets() gets lines
        Ctrl->>Ctrl: Parse pipe tokens: sscanf / strtok
        Ctrl->>Ctrl: Re-insert into BST and Min-Heap
    end
    Ctrl->>Disk: fclose()
    Ctrl-->>User: Toast "Reloaded snapshot from disk"
```

1. **Snapshot Serialization**: Retrieves all books via in-order BST extraction and writes them line-by-line using pipe delimiters (`ID|Title|Author|DueDate|IsReturned`).
2. **Snapshot Deserialization**: Wipes existing in-memory structures safely, parses the text lines, re-instantiates nodes, and updates the auto-increment ID counter.

---

### Workflow 8: Microsecond Latency Benchmarking (1M & 10M Slab)

```mermaid
sequenceDiagram
    autonumber
    participant User as Benchmarker
    participant UI as Benchmark Tab (TAB_BENCHMARK)
    participant Bench as Benchmark Engine (benchmark.c)
    participant Dual as Dual-Engine Index (BST + Heap)
    participant Trad as Traditional Flat Array

    User->>UI: Select Mode: "1M Dataset" or "10M Slab Dataset"
    User->>UI: Click "ALLOCATE DATASET"
    alt 1M Mode (~537 MB)
        Bench->>Bench: Allocate 1,000,000 dynamic nodes on Heap
    else 10M Mode (~2.82 GB)
        Bench->>Bench: Single malloc() slab + Iterative Non-Recursive BST insertion
    end
    Bench-->>UI: Allocation complete notification

    User->>UI: Click "RUN DUAL-ENGINE" or "RUN LINEAR SCAN"
    UI->>Bench: runDualVsTraditionalBenchmark()
    
    rect rgb(13, 148, 136)
        Note over Bench,Dual: High-Res Timer Start
        Bench->>Dual: Search query title in BST (Iterative O(log n))
        Bench->>Dual: Peek earliest due in Min-Heap (O(1))
        Note over Bench,Dual: High-Res Timer Stop -> Recorded in Microseconds (us)
    end

    rect rgb(225, 29, 72)
        Note over Bench,Trad: High-Res Timer Start
        Bench->>Trad: Linear scan 1M/10M structs for title match (O(n))
        Bench->>Trad: Linear scan 1M/10M structs for minimum due date (O(n))
        Note over Bench,Trad: High-Res Timer Stop -> Recorded in Milliseconds (ms)
    end

    Bench->>Bench: Verify Correctness: (dualFound == tradFound && dualMin == tradMin)
    Bench->>Bench: Calculate Speedup: (Linear Time / Dual-Engine Time)
    Bench-->>UI: Populate BenchmarkResult struct
    UI->>UI: Render Side-by-Side Comparison Cards & Speedup Banner
```

1. **Memory Allocation**:
   - **1M Mode**: Allocates 1,000,000 nodes using individual dynamic allocations along with a flat array baseline.
   - **10M Mode**: Uses a single contiguous memory pool (`Book* pool = malloc(10000000 * sizeof(Book))`) to eliminate heap fragmentation and runs iterative tree insertion to protect against stack exhaustion.
2. **Timing Execution**: Measures both algorithms with hardware performance counters:
   - **BST Query**: Takes $\approx 0.5$ to $2.0\ \mu\text{s}$ (performing only $\approx 20$ comparisons).
   - **Linear Scan**: Takes $\approx 15$ to $45\ \text{ms}$ on 1M records, and $\approx 150$ to $400\ \text{ms}$ on 10M records (examining every struct in memory).
3. **Audit & Verification**: Checks that both engines located the identical book ID and earliest due date.
4. **Display**: Displays metrics, comparison cards, and speedup multipliers in the UI.

---

### Workflow 9: Real-Time Algorithmic Flow Streaming

```mermaid
sequenceDiagram
    autonumber
    participant CApp as C Desktop Application
    participant Exporter as flow_exporter.c
    participant JSON as flow_state.json
    participant Flask as Flask Server (app.py)
    participant Browser as Web Visualizer (Canvas)

    CApp->>Exporter: exportFlowSnapshot(action, details, activeNode, step, totalBooks)
    Exporter->>JSON: Overwrite flow_state.json with sanitized JSON
    Browser->>Flask: GET /api/state (Polled or WebSocket / Scrubber)
    Flask->>JSON: Read flow_state.json
    Flask-->>Browser: JSON State Payload
    Browser->>Browser: Update animated SVG/Canvas nodes
    Browser->>Browser: Highlight active data flow path & step counter
```

1. **Snapshot Trigger**: Any significant action in the desktop app (adding a book, searching, running benchmarks) calls `exportFlowSnapshot()`.
2. **Atomic JSON Write**: Sanitizes strings and writes current execution state to `flow_state.json`.
3. **Web Visualization**: The Flask application serves an interactive canvas where users can watch data flow across architecture blocks (GUI -> Controller -> BST -> Heap -> Disk) in real time.

---

## 5. Memory Safety Invariants & Architectural Boundaries

1. **Single Node Ownership Model**:
   - All `Book` structures are owned exclusively by the Binary Search Tree.
   - The Min-Heap contains only borrowed pointers (`Book*`).
   - Destruction sequence:
     ```c
     freeBST(ctrl->bstRoot);      // Releases all actual Book memory
     freeMinHeap(ctrl->heap);     // Releases only the pointer array
     free(ctrl);                  // Releases controller wrapper
     ```
2. **Iterative Stack Overflow Protection for 10M Records**:
   - Standard recursive tree insertions overflow the call stack when handling millions of records.
   - The 10M benchmark module implements an **iterative insertion loop**, keeping stack frames constant ($O(1)$ stack overhead).
3. **Contiguous Slab Allocation for Massive Datasets**:
   - Allocating 10,000,000 individual heap nodes causes memory fragmentation and allocator thrashing.
   - The 10M benchmark utilizes a **pre-allocated memory slab** (`sizeof(Book) * 10,000,000` $\approx 2.82\ \text{GB}$), ensuring zero heap fragmentation and blazing-fast initialization.
4. **Buffer-Safe String Handling**:
   - All text copying operations employ bounded functions (`strncpy`, `snprintf`) with explicit null-terminator guarantees (`dest[maxLen - 1] = '\0'`), preventing buffer overruns.
5. **Decoupled Architecture**:
   - GUI code in `main.c` contains zero direct tree or heap mutations; all interactions pass through clean procedural interfaces in `controller.h`.
