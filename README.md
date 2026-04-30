# Algeria History Database
## NSCS – Algorithms and Dynamic Data Structures (S2 2025/2026)

---

## Project Structure

```
algeria_db/
├── Makefile
├── README.md
├── data/
│   └── algeria_history.txt      ← the database file
├── include/
│   ├── types.h                  ← all struct definitions + utils declarations
│   ├── file_parser.h            ← file format + parsing declarations
│   ├── list.h                   ← linked list, bidirectional, circular, queue
│   ├── stack.h                  ← stack module
│   ├── tree.h                   ← BST module
│   └── recursion.h              ← recursion module
└── src/
    ├── main.c                   ← interactive menu
    ├── utils.c                  ← Date helpers, string helpers
    ├── file_parser.c            ← file I/O and parsing
    ├── list.c                   ← all linked list + queue functions
    ├── stack.c                  ← all stack functions
    ├── tree.c                   ← all BST functions
    └── recursion.c              ← all recursive functions
```

---

## Database File Format (`data/algeria_history.txt`)

```
# Lines starting with # are comments

# PERSONALITY LINE:
#   NAME = DEFINITION {YYYY-MM-DD} {YYYY-MM-DD}
#   First {} = Date of Birth, Second {} = Date of Death
Emir Abdelkader = Algerian national hero... {1808-09-06} {1883-05-26}

# EVENT LINE:
#   EVENT NAME : YYYY-MM-DD
Battle of Algiers : 1956-09-30

# STANDALONE DATE LINE:
#   {YYYY-MM-DD}
{1954-11-01}
```

---

## Build & Run

```bash
# Build
make

# Run
./algeria_db
# or
make run

# Clean
make clean
```

Requires: `gcc`, `make` (both standard on any Linux distro).

---

## Data Structures Used

| Structure         | Purpose                                          |
|-------------------|--------------------------------------------------|
| `TList`           | Singly linked list of personalities (name+def)  |
| `TListDate`       | Singly linked list of personalities (name+dates)|
| `TListEvent`      | Singly linked list of events                     |
| `TBiList`         | Doubly (bidirectional) linked list (merged)      |
| `TCircList`       | Circular singly linked list (merged)             |
| `TStack`          | Linked stack                                     |
| `TQueue`/`TQNode` | Linked queue with front/rear pointers            |
| `TTree`           | Binary Search Tree (sorted by name)              |

---

## Extending for Raylib GUI

All data structures and logic are in separate `.c`/`.h` modules.
In your Raylib frontend, simply:

```c
#include "types.h"
#include "file_parser.h"
#include "list.h"
#include "stack.h"
#include "tree.h"
#include "recursion.h"
```

Then call the same functions used in `main.c` to populate and query the database, and render the results using Raylib's drawing primitives.

Compile your Raylib frontend alongside these modules:

```bash
gcc -Wall -Iinclude src/utils.c src/file_parser.c src/list.c \
    src/stack.c src/tree.c src/recursion.c \
    your_gui.c -o algeria_gui -lraylib -lm
```
