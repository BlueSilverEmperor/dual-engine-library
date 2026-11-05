import os
from flask import Flask, render_template, jsonify, abort

app = Flask(__name__, template_folder='templates')

# Root directory of the C project (one level up from flow_visualizer)
BASE_C_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

MODULES_DATA = [
    {
        "id": "gui",
        "name": "1. Main Loop & Raylib GUI",
        "file": "main.c",
        "badge": "60 FPS Raylib UI",
        "color": "#0D9488",
        "desc": "Handles resizable window rendering, theme palette state (Light/Dark), user form inputs, button click events, toast notifications, and interactive calendar popups.",
        "key_functions": [
            "main()", "DrawCustomInput()", "DrawExecutiveButton()", 
            "DrawCalendarPickerModal()", "UpdateCustomInput()", "showToast()"
        ],
        "complexity": "O(1) per frame event"
    },
    {
        "id": "controller",
        "name": "2. Central Library Controller",
        "file": "controller.c",
        "badge": "Business Logic Bridge",
        "color": "#3B82F6",
        "desc": "Serves as the central state bridge between the Raylib GUI and backend data structures. Coordinates single-allocation book creation and dual insertion into BST and Min-Heap.",
        "key_functions": [
            "initController()", "addBookController()", "searchBooksSubstringController()", 
            "checkOverdueController()", "returnBookController()", "freeController()"
        ],
        "complexity": "O(log n) catalog / O(log n) heap"
    },
    {
        "id": "bst",
        "name": "3. BST Catalog Engine",
        "file": "bst.c",
        "badge": "Alphabetical Tree Index",
        "color": "#10B981",
        "desc": "Stores Book records in a Binary Search Tree ordered case-insensitively by title. Owns the single dynamic memory allocation for Book nodes.",
        "key_functions": [
            "createBook()", "insertBST()", "searchBST()", 
            "searchSubstringBST()", "searchBSTById()", "inorderBSTToArray()", "freeBST()"
        ],
        "complexity": "O(log n) Search & Insert / O(n) In-order Traversal"
    },
    {
        "id": "heap",
        "name": "4. Min-Heap Overdue Queue",
        "file": "heap.c",
        "badge": "Priority Queue Engine",
        "color": "#F43F5E",
        "desc": "Priority queue storing zero-copy Book* pointer references ordered by integer due date (YYYYMMDD). Ignores returned books (isReturned == true) during overdue evaluations.",
        "key_functions": [
            "initMinHeap()", "insertMinHeap()", "bubbleUp()", 
            "minHeapify()", "peekMinHeap()", "extractMinHeap()", "getOverdueBooks()", "freeMinHeap()"
        ],
        "complexity": "O(1) Peek Root / O(log n) Bubble Up & Heapify"
    },
    {
        "id": "persistence",
        "name": "5. Persistence & Disk I/O",
        "file": "persistence.c",
        "badge": "Pipe-Delimited Serialization",
        "color": "#8B5CF6",
        "desc": "Handles file serialization to disk (library_data.txt) using pipe-delimited format (ID|Title|Author|DueDate|IsReturned). Auto-loads on application startup.",
        "key_functions": [
            "saveLibraryData()", "loadLibraryData()", "writeBSTNodeToFile()"
        ],
        "complexity": "O(n) Disk Save & Load"
    }
]

FLOW_PIPELINES = [
    {
        "step": 1,
        "title": "User Form Input & Action Trigger",
        "module": "main.c",
        "function": "DrawExecutiveButton() / UpdateCustomInput()",
        "description": "User enters book metadata (Title, Author, Due Date via Calendar) and clicks '+ ADD TO CATALOG & HEAP' or 'RETURN'.",
        "data_state": "Input strings validated & parsed: title='Clean Code', author='Robert Martin', dueDate=20260910"
    },
    {
        "step": 2,
        "title": "Controller Request Dispatch",
        "module": "controller.c",
        "function": "addBookController() / returnBookController()",
        "description": "Controller assigns auto-incrementing ID if needed and invokes createBook() to allocate the node.",
        "data_state": "Book struct allocated in heap memory: id=103, isReturned=false"
    },
    {
        "step": 3,
        "title": "BST Alphabetical Tree Insertion",
        "module": "bst.c",
        "function": "insertBST()",
        "description": "Inserts the Book node into the Binary Search Tree using title string comparison. BST owns node memory.",
        "data_state": "Node placed in BST based on strcmp(newBook->title, root->title)"
    },
    {
        "step": 4,
        "title": "Zero-Copy Min-Heap Pointer Insertion",
        "module": "heap.c",
        "function": "insertMinHeap() -> bubbleUp()",
        "description": "Pushes Book* pointer reference into Min-Heap dynamic array. Runs bubbleUp() to maintain due date min-heap order.",
        "data_state": "MinHeap.books[size] = Book* reference; bubbleUp() compares dueDate (YYYYMMDD)"
    },
    {
        "step": 5,
        "title": "Real-Time GUI Re-render & Disk Persistence",
        "module": "persistence.c",
        "function": "saveController() -> saveLibraryData()",
        "description": "GUI performs in-order BST traversal for 60 FPS table render; persistence engine flushes snapshot to library_data.txt.",
        "data_state": "Disk output: 103|Clean Code|Robert Martin|20260910|0"
    }
]

@app.route('/')
def flow_dashboard():
    return render_template('flow.html', modules=MODULES_DATA, flow_steps=FLOW_PIPELINES)

@app.route('/archify')
@app.route('/flow')
def archify_dashboard():
    return render_template('library_flow.html')

@app.route('/api/modules')
def get_modules():
    return jsonify({"status": "success", "modules": MODULES_DATA})

@app.route('/api/flow_steps')
def get_flow_steps():
    return jsonify({"status": "success", "steps": FLOW_PIPELINES})

@app.route('/api/c_code/<filename>')
def get_c_code(filename):
    # Restrict to valid C source/header files in root project directory
    allowed_files = {
        'main.c', 'bst.c', 'bst.h', 'heap.c', 'heap.h', 
        'persistence.c', 'persistence.h', 'controller.c', 'controller.h',
        'flow_exporter.c', 'flow_exporter.h', 'benchmark.c', 'benchmark.h', 'README.md'
    }
    if filename not in allowed_files:
        return jsonify({"status": "error", "message": "Access denied. Only C codebase files allowed."}), 403
    
    file_path = os.path.join(BASE_C_DIR, filename)
    if not os.path.exists(file_path):
        return jsonify({"status": "error", "message": f"File '{filename}' not found."}), 404
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
        return jsonify({"status": "success", "filename": filename, "content": content})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

import json

@app.route('/api/state')
def get_state():
    possible_paths = [
        os.path.join(os.path.dirname(__file__), 'flow_state.json'),
        os.path.join(BASE_C_DIR, 'flow_state.json'),
        os.path.join(BASE_C_DIR, 'flow_visualizer', 'flow_state.json')
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            try:
                with open(path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    return jsonify(data)
            except Exception:
                pass
                
    return jsonify({
        "lastAction": "Idle",
        "details": "Waiting for C application activity...",
        "activeNode": "gui",
        "activeStep": 1,
        "timestamp": "Ready",
        "stats": {"totalBooks": 0}
    })

if __name__ == '__main__':
    print("=" * 70)
    print("  [*] Standalone C Code Flow Visualizer Server Running")
    print("  [*] URL: http://127.0.0.1:6400")
    print("=" * 70)
    app.run(host='127.0.0.1', port=6400, debug=True)
