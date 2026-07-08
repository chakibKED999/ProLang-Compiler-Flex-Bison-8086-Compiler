# 🚀 ProLang Compiler

A complete **end-to-end compiler** for a custom procedural programming language called **ProLang**, built using **Flex**, **Bison**, **C**, **Python**, and **8086 Assembly**.

The compiler implements every major phase of the compilation process—from lexical analysis to optimized machine code generation—and includes a modern graphical IDE for writing, compiling, and visualizing programs.

---

## ✨ Features

- 🔍 Lexical Analysis using **Flex**
- 🌳 Syntax Analysis using **Bison (LALR Parser)**
- 🧠 Semantic Analysis
- 📚 Hash Table Symbol Table
- ⚙️ Intermediate Code Generation (Quadruplets)
- ⚡ Multi-pass Code Optimization
- 💾 8086 Assembly Code Generation
- 🔢 Fixed-Point Floating Point Emulation
- 🖥️ Integrated Tkinter GUI
- 🚀 One-click Build & Compilation
- 📄 Compilation Reports
- 🛠️ EMU8086 Integration

---

# 🏗️ Compiler Architecture

```
                 ProLang Source Code
                         │
                         ▼
             Lexical Analysis (Flex)
                         │
                         ▼
            Syntax Analysis (Bison)
                         │
                         ▼
            Semantic Verification
                         │
                         ▼
          Hash-Based Symbol Table
                         │
                         ▼
      Intermediate Representation
             (Quadruplets)
                         │
                         ▼
          Optimization Passes
                         │
                         ▼
          8086 Assembly Generator
                         │
                         ▼
                output.asm
```

---

# 📖 Compilation Pipeline

## 1️⃣ Lexical Analysis

**File**

```
lexical.l
```

Responsibilities

- Token recognition
- Keywords
- Identifiers
- Integer constants
- Float constants
- Operators
- Separators
- Single-line comments
- Multi-line comments
- Lexical error detection
- Line & column tracking

---

## 2️⃣ Syntax & Semantic Analysis

**File**

```
syn.y
```

The parser validates the grammar while simultaneously performing semantic verification.

Supported features

- Variable declarations
- Constant declarations
- Arrays
- Assignments
- Arithmetic expressions
- Boolean expressions
- if / else
- while loops
- for loops
- Input / Output
- Nested blocks

Semantic verification includes

- Duplicate declarations
- Undeclared identifiers
- Type compatibility
- Constant modification detection
- Array validation

---

## 3️⃣ Symbol Table

**File**

```
ts.h
```

Implemented as a **Hash Table**.

```
HASH_SIZE = 101
```

Each entry stores

- Identifier
- Category
- Type
- Value
- Array Size
- Memory Information

Operations

- Insert
- Search
- Update
- Duplicate detection

---

## 4️⃣ Intermediate Code Generation

Files

```
quad.c
quad.h
```

The compiler generates a machine-independent Intermediate Representation using **Quadruplets**.

Example

```
(+, a, b, T1)
(*, T1, c, T2)
(:=, T2, -, result)
```

This intermediate representation simplifies optimization and machine code generation.

---

## 5️⃣ Intermediate Code Optimization

Files

```
optim.c
optim.h
```

Implemented optimization passes

- Copy Propagation
- Expression Propagation
- Common Subexpression Elimination
- Constant Propagation
- Algebraic Simplification
- Dead Code Elimination

The optimizer executes repeatedly until no additional optimization is possible.

---

## 6️⃣ 8086 Assembly Generation

Files

```
codegen.c
codegen.h
```

The backend converts optimized quadruplets into executable **8086 Assembly**.

Supports

- Arithmetic operations
- Assignments
- Conditional branches
- Loops
- Labels
- Input / Output routines
- Temporary variables

---

## 7️⃣ Floating-Point Support

The Intel 8086 processor has no hardware Floating Point Unit (FPU).

To support decimal numbers, the compiler uses **Fixed-Point Arithmetic**.

```
Stored Value = Real Value × 256
```

Example

```
3.5

↓

896
```

Arithmetic operations are performed using integer registers and converted back when needed.

---

## 8️⃣ Visual Compiler Studio

**File**

```
visual_compiler.py
```

Built with **Tkinter**, the IDE provides

- Code editor
- Syntax highlighting
- One-click compilation
- Automatic Flex/Bison build
- Assembly preview
- Error reporting
- EMU8086 launcher
- Compilation logs

---

# 📂 Project Structure

```
ProLang-Compiler/
│
├── lexical.l
├── syn.y
├── ts.h
│
├── quad.c
├── quad.h
│
├── optim.c
├── optim.h
│
├── codegen.c
├── codegen.h
│
├── visual_compiler.py
│
├── output.asm
├── rapport_visualisation.txt
│
└── README.md
```

---

# 🛠 Technologies

- C
- Python
- Flex
- Bison
- Tkinter
- GCC (MinGW)
- 8086 Assembly
- EMU8086

---

# 🚀 Getting Started

## Prerequisites

Install

- GCC (MinGW)
- Flex
- Bison
- Python 3
- EMU8086 (optional)

---

## Run

```bash
python visual_compiler.py
```

Inside the application click

```
Tout lancer
```

The IDE automatically

- Generates the scanner
- Generates the parser
- Compiles the compiler
- Produces optimized quadruplets
- Generates the final `output.asm`

---

# 📤 Generated Files

```
lex.yy.c
syn.tab.c
syn.tab.h
prolang.exe
output.asm
rapport_visualisation.txt
```

---

# 📊 Technical Highlights

| Feature | Implementation |
|----------|----------------|
| Compiler Language | ProLang |
| Scanner | Flex |
| Parser | Bison (LALR) |
| Symbol Table | Hash Table |
| Intermediate Code | Quadruplets |
| Optimization | Multi-pass |
| Backend | 8086 Assembly |
| Floating Point | Fixed-Point ×256 |
| GUI | Tkinter |
| Build | Flex + Bison + GCC |

---

# 🔬 Design Highlights

### Fixed-Point Arithmetic

Since the Intel 8086 lacks floating-point hardware, decimal values are internally represented using fixed-point arithmetic (×256), enabling floating-point computations with integer instructions.

### Branch Inversion

Conditional statements and loops are translated using inverted branch logic (e.g., `>` → `BLE`) to generate cleaner and more efficient intermediate code without maintaining complex branch stacks.

### Multi-Pass Optimization

Optimization is performed iteratively until convergence, significantly reducing redundant computations, temporary variables, and unnecessary assembly instructions.

---

# 📈 Future Improvements

- Graph-coloring register allocation
- Linear scan register allocation
- Constant folding
- Loop invariant code motion
- Peephole optimization
- Multi-dimensional arrays
- User-defined functions
- String support
- Better syntax error recovery
- x86/x64 backend generation

---

# 🎓 Academic Project

This project was developed as part of the **Master 1 – Compilation** course during the **2025–2026** academic year. It implements a complete compiler pipeline for the custom **ProLang** language, covering lexical analysis, syntax and semantic analysis, symbol table management, intermediate representation, optimization, and 8086 assembly code generation.
