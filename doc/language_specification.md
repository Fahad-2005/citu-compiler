# Citu Language Specification

## 1. Core Language Rules & Scoping
`Citu` is a procedural programming language designed for structured program execution. It strictly follows the **ANSI C (C89) Scope Rule**:
- **Declaration-Before-Statement:** All variable declarations (`int32`, `float`, etc.) within any scope block (`shro` ... `khatam`) **must** be placed at the very top of the block, before any executable statements.
- **Block Delimiters:** Code blocks begin with `shro` and end with `khatam`.
- **Language Exclusions:** Object-oriented features (`classes`) and `do-while` loops are explicitly excluded from the language.

---

## 2. Supported Data Types
`Citu` supports fixed-width integers, floating-point numbers, and ASCII characters:

| Type Keyword | Description | Size |
| :--- | :--- | :--- |
| `int16` | Signed 16-bit integer | 2 Bytes |
| `int32` | Signed 32-bit integer | 4 Bytes |
| `uint16` | Unsigned 16-bit integer | 2 Bytes |
| `uint32` | Unsigned 32-bit integer | 4 Bytes |
| `float` | 32-bit floating-point number | 4 Bytes |
| `char` | Single byte ASCII character | 1 Byte |

---

## 3. Keywords & Syntax Mechanics

| Keyword | Purpose / C Equivalent | Example / Usage |
| :--- | :--- | :--- |
| `shro` | Starts a scope block (`{`) | `shro` |
| `khatam` | Ends a scope block (`}`) | `khatam` |
| `medalo` | Assignment operator (`=`) | `x medalo (i + 1);` |
| `agar` | Conditional branch (`if`) | `agar (a[i] == 0) shro ... khatam` |
| `magar` | Alternative branch (`else`) | `magar shro ... khatam` |
| `jabtak` | Iterative loop (`while`) | `jabtak (i < SIZE) shro ... khatam` |
| `nikal` | Exit loop (`break`) | `nikal;` |
| `return` | Return value from function | `return min_val;` |
| `struct` | User-defined data structure | `struct Point shro ... khatam` |

---

## 4. Example Programs

### Example 1: Find Minimum in an Array of 20 (`examples/ex1.citu`)
**What this program does:**
- Declares a 20-element `int32` array in `main()` and passes it to the `find_min` function.

```citu
int32 find_min(int32 arr[20]) shro
    int32 min_val;
    int32 i;

    min_val medalo arr[0];
    i medalo 1;

    jabtak (i < 20) shro
        agar (arr[i] < min_val) shro
            min_val medalo arr[i];
        khatam
        i medalo (i + 1);
    khatam

    return min_val;
khatam

int32 main() shro
    int32 numbers[20];
    int32 smallest;

    numbers[0] medalo 45;
    numbers[1] medalo 12;
    smallest medalo find_min(numbers);

    return smallest;
khatam
```

---

### Example 2: Pass/Fail Marks Checker (`examples/ex2.citu`)
**What this program does:**
- Takes a student's `marks` in `check_pass` and checks whether the score is `50` or above.

```citu
int32 check_pass(int32 marks) shro
    int32 result;

    agar (marks >= 50) shro
        result medalo 1;
    khatam
    magar shro
        result medalo 0;
    khatam

    return result;
khatam

int32 main() shro
    int32 score;
    int32 status;

    score medalo 75;
    status medalo check_pass(score);

    return status;
khatam
```

---

### Example 3: Sum of First N Numbers (`examples/ex3.citu`)
**What this program does:**
- Passes `10` from `main()` into `sum_numbers(int32 n)` to return total sum.

```citu
int32 sum_numbers(int32 n) shro
    int32 total;
    int32 i;

    total medalo 0;
    i medalo 1;

    jabtak (i <= n) shro
        total medalo (total + i);
        i medalo (i + 1);
    khatam

    return total;
khatam

int32 main() shro
    int32 ans;

    ans medalo sum_numbers(10);
    return ans;
khatam
```

---

## 5. Citu Language Grammar

### 5.1 Lexical Rules (Token Specifications)
The Lexical Analyzer scans `.citu` source files and converts character streams into the following token categories:

| Token Category | Token Name | Pattern / Lexeme |
| :--- | :--- | :--- |
| **Type Keywords** | `KW_INT16`, `KW_INT32`, `KW_UINT16`, `KW_UINT32`, `KW_FLOAT`, `KW_CHAR` | `int16`, `int32`, `uint16`, `uint32`, `float`, `char` |
| **Control Keywords** | `KW_SHRO`, `KW_KHATAM`, `KW_AGAR`, `KW_MAGAR`, `KW_JABTAK`, `KW_NIKAL`, `KW_RETURN`, `KW_STRUCT` | `shro`, `khatam`, `agar`, `magar`, `jabtak`, `nikal`, `return`, `struct` |
| **Assignment Operator** | `OP_MEDALO` | `medalo` |
| **Arithmetic Operators** | `OP_PLUS`, `OP_MINUS`, `OP_MUL`, `OP_DIV` | `+`, `-`, `*`, `/` |
| **Relational Operators** | `OP_EQ`, `OP_NEQ`, `OP_LT`, `OP_LTE`, `OP_GT`, `OP_GTE` | `==`, `!=`, `<`, `<=`, `>`, `>=` |
| **Member Access** | `OP_DOT` | `.` |
| **Delimiters** | `LPAREN`, `RPAREN`, `LBRACK`, `RBRACK`, `SEMI`, `COMMA` | `(`, `)`, `[`, `]`, `;`, `,` |
| **Identifiers** | `ID` | `[a-zA-Z_][a-zA-Z0-9_]*` |
| **Integer Literal** | `INT_LITERAL` | `[0-9]+` |
| **Float Literal** | `FLOAT_LITERAL` | `[0-9]+\.[0-9]+` |
| **Character Literal** | `CHAR_LITERAL` | `'[^']'` |
| **Whitespace & Comments** | *(Ignored)* | Spaces, `\t`, `\n`, `\r`, `//...`, `/*...*/` |

---

### 5.2 Syntax Rules (Context-Free Grammar)
The Context-Free Grammar (CFG) below defines valid syntactic structures in `Citu`. Notice that the `Block` production (`shro VarDeclList StmtList khatam`) structurally enforces the **ANSI C (C89) Scope Rule**: all variable declarations must appear before any executable statements.

```bnf
Program       -> DeclList

DeclList      -> Decl DeclList
               | ε

Decl          -> StructDecl
               | FuncDecl

StructDecl    -> struct ID shro VarDeclList khatam

FuncDecl      -> TypeSpec ID ( ParamList ) Block

ParamList     -> Param , ParamList
               | Param
               | ε

Param         -> TypeSpec ID
               | TypeSpec ID [ ]
               | TypeSpec ID [ INT_LITERAL ]

Block         -> shro VarDeclList StmtList khatam

VarDeclList   -> VarDecl VarDeclList
               | ε

VarDecl       -> TypeSpec ID ;
               | TypeSpec ID [ INT_LITERAL ] ;

TypeSpec      -> int16
               | int32
               | uint16
               | uint32
               | float
               | char
               | struct ID

StmtList      -> Stmt StmtList
               | ε

Stmt          -> AssignStmt
               | IfStmt
               | WhileStmt
               | BreakStmt
               | ReturnStmt
               | Block

AssignStmt    -> LValue medalo Expr ;

LValue        -> ID
               | ID [ Expr ]
               | ID . ID

IfStmt        -> agar ( Expr ) Block
               | agar ( Expr ) Block magar Block

WhileStmt     -> jabtak ( Expr ) Block

BreakStmt     -> nikal ;

ReturnStmt    -> return Expr ;
               | return ;

Expr          -> RelExpr

RelExpr       -> AddExpr RelOp AddExpr
               | AddExpr

RelOp         -> == | != | < | <= | > | >=

AddExpr       -> AddExpr + Term
               | AddExpr - Term
               | Term

Term          -> Term * Factor
               | Term / Factor
               | Factor

Factor        -> ( Expr )
               | LValue
               | FuncCall
               | INT_LITERAL
               | FLOAT_LITERAL
               | CHAR_LITERAL

FuncCall      -> ID ( ArgList )

ArgList       -> Expr , ArgList
               | Expr
               | ε
```

---

## 6. Git Repository Details
- **Student ID:** `BSCS23047`
- **Repository URL:** `https://github.com/Fahad-2005/citu-compiler.git`