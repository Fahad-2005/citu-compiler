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

## 5. Git Repository Details
- **Student ID:** `BSCS23047`
- **Repository URL:** `https://github.com/Fahad-2005/citu-compiler.git`