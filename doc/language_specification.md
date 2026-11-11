# Citu Language Specification

## 1. Core Language Rules & Scoping
`Citu` is a procedural programming language designed for structured program execution. It strictly follows the **ANSI C (C89) Scope Rule**:
* **Declaration-Before-Statement:** All variable declarations (`int32`, `float`, etc.) within any scope block (`shro` ... `khatam`) **must** be placed at the very top of the block, before any executable statements.
* **Block Delimiters:** Code blocks begin with `shro` and end with `khatam`.
* **Language Exclusions:** Object-oriented features (`classes`) and `do-while` loops are explicitly excluded from the language.

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
| `nikal` | Break / return control | `nikal;` |
