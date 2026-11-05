# Smart Library Book Inventory & Due-Date Tracker

.\library_app.exe -> executes the application


A high-performance desktop application written in **Pure C (C99)** for managing library catalog inventories and tracking overdue books in real time using a **Dual-Engine Data Structure Architecture** (Binary Search Tree + Min-Heap) and an interactive 60 FPS **Raylib** graphical user interface.

---

## 🌟 Key Features

- **Dual-Engine Data Structure Core**:
  - **BST Catalog Engine ($O(\log n)$)**: Stores `Book` records in a Binary Search Tree sorted alphabetically by title. Enables fast lookups and in-order $O(n)$ array extraction for rendering sorted inventory tables.
  - **Min-Heap Overdue Engine ($O(1)$ / $O(\log n)$)**: Priority Queue index ordered by integer due dates (`YYYYMMDD`), allowing immediate access to the earliest due items and high-speed overdue evaluations.
- **Modern Raylib Graphical UI**:
  - **Dual Theme Support**: Light Mode (Clean Paper & Sage Teal) and Dark Mode (Executive Dark Slate & Emerald).
  - **Typography Engine**: Crisp TrueType font rendering with `segoeui.ttf`.
  - **Tabbed Views**: In-Order Catalog Table, Priority Overdue Evaluator, and Title Search Inspector.
  - **Interactive Input Form**: Dynamic sidebar inputs with keyboard navigation and auto-incrementing ID assigner.
  - **Live Toast Alerts**: Auto-dismissing status notification banner system.
- **Disk Persistence Engine**:
  - Auto-loads existing inventory from pipe-delimited `library_data.txt` on startup.
  - Manual disk snapshot save/reload toolbar buttons and automatic snapshot flush upon exit.
- **Comprehensive Unit Verification**:
  - Standalone unit test suites for BST, Min-Heap, and Controller/Persistence modules.

---

## 🏛️ System Architecture & Data Flow

```
                      +-----------------------------------+
                      |        Library Controller         |
                      |     (Central Business Bridge)     |
                      +-----------------+-----------------+
                                        |
              +-------------------------+-------------------------+
              |                                                   |
              v                                                   v
    +-------------------+                               +-------------------+
    |  BST Catalog      |                               | Min-Heap Overdue  |
    |  (In-Order Tree)  |                               | (Priority Queue)  |
    +-------------------+                               +-------------------+
    | • O(log n) Search |                               | • O(1) Min Peek   |
    | • Title Sorting   |                               | • Due-Date Order  |
    | • In-Order Array  |                               | • O(log n) Extract|
    +-------------------+                               +-------------------+
              |                                                   |
              +-------------------------+-------------------------+
                                        |
                                        v
                      +-----------------------------------+
                      |    Persistence Engine (Disk I/O)  |
                      |       `library_data.txt`          |
                      +-----------------------------------+
```

### Time & Space Complexity Matrix

| Operation | Primary Data Structure | Time Complexity | Space Complexity | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Catalog Search** | Binary Search Tree | $O(\log n)$ avg / $O(n)$ worst | $O(1)$ auxiliary | Alphabetical lookup by book title |
| **Book Insertion** | BST + Min-Heap | $O(\log n)$ | $O(1)$ node alloc | Inserts node in BST and pointer into Min-Heap |
| **Earliest Due Peek** | Min-Heap | $O(1)$ | $O(1)$ | Fetches root element without heap mutation |
| **Overdue Check** | Min-Heap | $O(k \log n)$ ($k \le n$) | $O(k)$ output buffer | Evaluates heap priority queue up to reference date |
| **Catalog Traversal** | BST | $O(n)$ | $O(n)$ array output | In-order depth-first traversal for sorted view |
| **Disk Save / Load** | Persistence Engine | $O(n)$ | $O(n)$ file buffer | Pipe-delimited disk serialization and parsing |

---

## 📁 Codebase & File Structure

```
ds app/
├── bst.h / bst.c                   # Module 1: Binary Search Tree catalog engine
├── heap.h / heap.c                 # Module 2: Min-Heap priority queue overdue engine
├── persistence.h / persistence.c   # Module 3A: File serialization & pipe parsing
├── controller.h / controller.c     # Module 3B: Central controller & state manager
├── main.c                          # Module 4: Raylib 60 FPS graphical UI & main loop
├── raygui.h                        # Immediate-mode GUI component library header
├── segoeui.ttf                     # High-legibility Segoe UI TrueType font
├── library_data.txt                # Auto-generated disk persistence snapshot
├── Dual_Engine_Library_Architecture.pdf # Visual architecture blueprint document
├── test_bst.c                      # Standalone unit test suite for Module 1 (BST)
├── test_heap.c                     # Standalone unit test suite for Module 2 (Min-Heap)
├── test_controller.c               # Standalone unit test suite for Module 3 (Persistence/Controller)
├── library_app.exe                 # Compiled desktop application binary
└── README.md                       # Complete technical documentation
```

## 🚀 Program Execution & Paths

### Application Executable File Paths
- **Main GUI Application Binary**: `c:\Users\sridh\Desktop\ds app\library_app.exe` (or `./library_app.exe` relative to project directory)
- **Data Persistence File**: `c:\Users\sridh\Desktop\ds app\library_data.txt`
- **Unit Test Binaries**:
  - `c:\Users\sridh\Desktop\ds app\test_bst.exe`
  - `c:\Users\sridh\Desktop\ds app\test_heap.exe`
  - `c:\Users\sridh\Desktop\ds app\test_controller.exe`

---

## 🛠️ Compilation & Execution Guide

### Prerequisites
- **C Compiler**: GCC (MinGW-w64 / MSYS2 UCRT64 on Windows, or GCC on Linux/macOS).
- **Raylib 5.5**: Installed via MSYS2 (`pacman -S mingw-w64-ucrt-x86_64-raylib`) or system package manager.

---

### 1. Compiling & Launching the Main Application

#### **Run Directly in PowerShell**
```powershell
# Method 1: Run directly from inside the project directory (Recommended)
.\library_app.exe

# Method 2: Run using absolute path (PowerShell requires & before quoted paths containing spaces)
& "c:\Users\sridh\Desktop\ds app\library_app.exe"
```

#### **Compile and Run from Source (Windows MSYS2 UCRT64 / MinGW GCC)**
```powershell
gcc main.c bst.c heap.c persistence.c controller.c -lraylib -lopengl32 -lgdi32 -lwinmm -o library_app.exe
.\library_app.exe
```

#### **Linux / POSIX Systems**
```bash
gcc main.c bst.c heap.c persistence.c controller.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o library_app
./library_app
```

---

### 2. Running Standalone Unit Test Suites

Each module contains an isolated unit test suite validating memory safety, boundary cases, and algorithmic correctness:

- **Module 1 (BST Catalog Unit Tests):**
  ```powershell
  gcc test_bst.c bst.c -o test_bst.exe && .\test_bst.exe
  ```

- **Module 2 (Min-Heap Overdue Unit Tests):**
  ```powershell
  gcc test_heap.c heap.c bst.c -o test_heap.exe && .\test_heap.exe
  ```

- **Module 3 (Persistence & Controller Unit Tests):**
  ```powershell
  gcc test_controller.c controller.c persistence.c heap.c bst.c -o test_controller.exe && .\test_controller.exe
  ```

---

## 🎨 Interactive GUI Features & Walkthrough

| Interface Element | User Action | System Behavior |
| :--- | :--- | :--- |
| **Add Book Sidebar** | Input Title, Author, Due Date | Validates fields and inserts book into both BST and Min-Heap simultaneously. |
| **Catalog Tab** | View Table | Renders all inventory books sorted alphabetically by title with alternating row colors. |
| **Overdue Tab** | Set Cutoff Date & Evaluate | Filters Min-Heap for books due on/before cutoff date and displays warning indicators. |
| **Search Tab** | Title Query Lookup | Performs $O(\log n)$ case-insensitive search and highlights matching book metadata card. |
| **Header Bar** | Theme & Disk Controls | Toggle between Light/Dark modes, manual save to `library_data.txt`, or reload disk data. |

---

## 💾 Disk Persistence Schema

Inventory records are stored in `library_data.txt` using pipe-delimited encoding (`ID|Title|Author|DueDate|IsReturned`):

```text
101|The C Programming Language|Kernighan & Ritchie|20260920|0
102|Introduction to Algorithms|Cormen, Leiserson|20261015|0
103|Clean Code|Robert C. Martin|20260910|1
104|Design Patterns|Gang of Four|20260925|0
```

---

## ⚡ Memory Safety & Performance Principles

1. **Single Allocation Node Ownership**:
   - `Book` nodes are allocated dynamically once via `createBook()`.
   - The Binary Search Tree owns the node memory and recursively releases all allocated nodes upon calling `freeBST()`.
2. **Zero-Copy Pointer Priority Queue**:
   - The Min-Heap priority queue stores references to `Book*` pointers, avoiding struct cloning and memory overhead.
   - Calling `freeMinHeap()` releases the heap array container without touching underlying `Book` pointers.
3. **Clean Application Shutdown**:
   - `freeController()` systematically flushes pending inventory changes to disk, frees BST nodes, and destroys heap structures cleanly without leaks.
