🚀 Mini-ProLang Compiler Studio

A complete compiler toolchain for a custom programming language called ProLang, developed using Flex, Bison, C, Python, and 8086 Assembly.

The project implements every major compilation phase—from lexical analysis to optimized machine code generation—while providing an integrated graphical IDE for writing, compiling, and visualizing the compilation process.

✨ Features
✅ Lexical Analysis with Flex
✅ Syntax Analysis using Bison (LALR Parser)
✅ Semantic Analysis
✅ Hash Table Symbol Table
✅ Intermediate Code Generation (Quadruplets)
✅ Multi-pass Optimization Engine
✅ 8086 Assembly Code Generation
✅ Fixed-Point Floating Point Emulation
✅ Integrated Python GUI
✅ Automatic Build System
✅ EMU8086 Integration
🏗️ Compiler Architecture
            Source Code (ProLang)
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
          Symbol Table (Hash)
                     │
                     ▼
      Intermediate Code (Quads)
                     │
                     ▼
      Optimization Passes
                     │
                     ▼
      8086 Assembly Generator
                     │
                     ▼
           Executable Assembly
📚 Compilation Pipeline
1. Lexical Analysis (Flex)

File

lexical.l

Responsibilities

Tokenization
Keyword recognition
Identifier validation
Numeric literals
Operators
Separators
Single-line comments (%%)
Multi-line comments (//* ... *//)
Line and column tracking
Lexical error reporting
2. Syntax & Semantic Analysis (Bison)

File

syn.y

Responsibilities

LALR parsing
Grammar validation
Variable declarations
Constant declarations
Array declarations
Assignment validation
Control-flow parsing
Type checking
Undeclared identifier detection
Duplicate declaration detection
Constant modification prevention

Supported statements

Assignment
if / else
while
for
input
output
3. Symbol Table

File

ts.h

Implementation

Hash Table
HASH_SIZE = 101

Each symbol stores

Name
Category
Type
Initial Value
Array Size
Memory Information

Supports

Insertion
Lookup
Duplicate detection
4. Intermediate Representation

Files

quad.c
quad.h

The compiler generates Quadruplets in the form

(Op, Arg1, Arg2, Result)

Example

(+, a, b, T1)
(*, T1, c, T2)
(:=, T2, -, x)
5. Optimization Engine

Files

optim.c
optim.h

Implemented optimizations

Copy Propagation
Expression Propagation
Common Subexpression Elimination
Constant Propagation
Algebraic Simplification
Dead Code Elimination

The optimizer runs repeatedly until no additional changes can be applied.

6. Code Generation

Files

codegen.c
codegen.h

Generates

8086 Assembly

Features

Arithmetic instructions
Conditional jumps
Loop generation
Input / Output routines
Variable allocation
Temporary variable handling
7. Floating-Point Support

The Intel 8086 processor has no Floating Point Unit (FPU).

To overcome this limitation, floating-point values are represented using fixed-point arithmetic.

Stored Value = Real Value × 256

Example

3.5

↓

896

Operations are performed on integers and converted back when necessary.

8. Graphical IDE

File

visual_compiler.py

Built with

Tkinter

Features

Code editor
Syntax highlighting
One-click compilation
Automatic Flex/Bison build
Error visualization
Assembly preview
EMU8086 integration
Compilation reports
📁 Project Structure
Mini-ProLang-Compiler/
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
⚙️ Technologies Used
C
Python
Flex
Bison
8086 Assembly
Tkinter
GCC (MinGW)
EMU8086
🚀 Running the Project

Install

GCC (MinGW)
Flex
Bison
EMU8086 (optional)

Launch

python visual_compiler.py

Inside the IDE

Click "Tout lancer"

The application automatically

Generates the scanner
Generates the parser
Builds the compiler
Produces optimized quadruplets
Generates output.asm
📤 Generated Outputs
lex.yy.c
syn.tab.c
syn.tab.h
prolang.exe
output.asm
rapport_visualisation.txt
📊 Technical Highlights
Feature	Implementation
Language	ProLang
Parser	Bison (LALR)
Scanner	Flex
Symbol Table	Hash Table (101 buckets)
Intermediate Code	Quadruplets
Optimization	Multi-pass
Target Architecture	Intel 8086
Floating Point	Fixed-Point (×256)
GUI	Tkinter
Build System	Flex + Bison + GCC
🔬 Interesting Design Decisions
Fixed-Point Arithmetic

Because the 8086 lacks hardware floating-point support, floating-point values are internally scaled by 256, allowing arithmetic operations to be executed using standard integer registers.

Branch Inversion

Instead of maintaining complex jump stacks, conditional expressions are translated using inverted branch logic (e.g., > becomes BLE), resulting in simpler and more linear quadruplet generation.

Optimization Until Convergence

Optimization passes execute iteratively until no further transformations can be applied, reducing unnecessary temporaries and generating more efficient assembly output.

📈 Future Improvements
Register allocation using graph coloring
Linear scan register allocation
Function definitions and calls
Multi-dimensional arrays
String data type
Better syntax error recovery
Constant folding
Loop invariant code motion
Peephole optimization
Additional target architectures (x86/x64)
📄 Academic Context

This project was developed for the Master 1 – Compilation module at Université des Sciences et de la Technologie Houari Boumédiène during the 2025–2026 academic year. It implements the complete compiler pipeline specified in the course project, including lexical analysis, parsing, semantic analysis, symbol table management, intermediate code generation, optimization, and 8086 assembly generation.
