#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

enum TokenType {
    // Type & Control Keywords
    KW_INT16, KW_INT32, KW_UINT16, KW_UINT32, KW_FLOAT, KW_CHAR,
    KW_SHRO, KW_KHATAM, KW_AGAR, KW_MAGAR, KW_JABTAK, KW_NIKAL, KW_RETURN, KW_STRUCT,

    // Operators & Delimiters
    OP_MEDALO, OP_PLUS, OP_MINUS, OP_MUL, OP_DIV,
    OP_EQ, OP_NEQ, OP_LT, OP_LTE, OP_GT, OP_GTE, OP_DOT,
    LPAREN, RPAREN, LBRACK, RBRACK, SEMI, COMMA,

    // Identifiers, Literals & EOF
    ID, INT_LITERAL, FLOAT_LITERAL, CHAR_LITERAL, TOKEN_EOF
};

struct Token {
    TokenType type;   // Token category
    string lexeme;    // Exact matched text
    int line;         // 1-based line number
    int column;       // 1-based column offset
};

string tokenTypeToString(TokenType type);

#endif // TOKEN_H