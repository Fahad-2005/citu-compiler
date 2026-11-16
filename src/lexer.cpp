#include "../include/lexer.h"
#include <cctype>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

string tokenTypeToString(TokenType type) {
    switch (type) {
        case KW_INT16:      return "KW_INT16";
        case KW_INT32:      return "KW_INT32";
        case KW_UINT16:     return "KW_UINT16";
        case KW_UINT32:     return "KW_UINT32";
        case KW_FLOAT:      return "KW_FLOAT";
        case KW_CHAR:       return "KW_CHAR";
        case KW_SHRO:       return "KW_SHRO";
        case KW_KHATAM:     return "KW_KHATAM";
        case KW_AGAR:       return "KW_AGAR";
        case KW_MAGAR:      return "KW_MAGAR";
        case KW_JABTAK:     return "KW_JABTAK";
        case KW_NIKAL:      return "KW_NIKAL";
        case KW_RETURN:     return "KW_RETURN";
        case KW_STRUCT:     return "KW_STRUCT";
        case OP_MEDALO:     return "OP_MEDALO";
        case OP_PLUS:       return "OP_PLUS";
        case OP_MINUS:      return "OP_MINUS";
        case OP_MUL:        return "OP_MUL";
        case OP_DIV:        return "OP_DIV";
        case OP_EQ:         return "OP_EQ";
        case OP_NEQ:        return "OP_NEQ";
        case OP_LT:         return "OP_LT";
        case OP_LTE:        return "OP_LTE";
        case OP_GT:         return "OP_GT";
        case OP_GTE:        return "OP_GTE";
        case OP_DOT:        return "OP_DOT";
        case LPAREN:        return "LPAREN";
        case RPAREN:        return "RPAREN";
        case LBRACK:        return "LBRACK";
        case RBRACK:        return "RBRACK";
        case SEMI:          return "SEMI";
        case COMMA:         return "COMMA";
        case ID:            return "ID";
        case INT_LITERAL:   return "INT_LITERAL";
        case FLOAT_LITERAL: return "FLOAT_LITERAL";
        case CHAR_LITERAL:  return "CHAR_LITERAL";
        case TOKEN_EOF:     return "TOKEN_EOF";
        default:            return "UNKNOWN";
    }
}

Lexer::Lexer(const string& src)
    : source(src), pos(0), line(1), column(1) {
    keywords = {
        {"int16",  KW_INT16},
        {"int32",  KW_INT32},
        {"uint16", KW_UINT16},
        {"uint32", KW_UINT32},
        {"float",  KW_FLOAT},
        {"char",   KW_CHAR},
        {"shro",   KW_SHRO},
        {"khatam", KW_KHATAM},
        {"agar",   KW_AGAR},
        {"magar",  KW_MAGAR},
        {"jabtak", KW_JABTAK},
        {"nikal",  KW_NIKAL},
        {"return", KW_RETURN},
        {"struct", KW_STRUCT},
        {"medalo", OP_MEDALO}
    };
}

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

char Lexer::peek(size_t offset) const {
    if (pos + offset >= source.size()) {
        return '\0';
    }
    return source[pos + offset];
}

char Lexer::advance() {
    char current = source[pos++];
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();

        // Whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        }
        // Single-line comment: // ...
        else if (c == '/' && peek(1) == '/') {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        }
        // Multi-line comment: /* ... */
        else if (c == '/' && peek(1) == '*') {
            int commentLine = line;
            int commentCol = column;
            advance(); // consume '/'
            advance(); // consume '*'

            bool closed = false;
            while (!isAtEnd()) {
                if (peek() == '*' && peek(1) == '/') {
                    advance(); // consume '*'
                    advance(); // consume '/'
                    closed = true;
                    break;
                }
                advance();
            }

            if (!closed) {
                throw runtime_error(
                    "Lexical Error at line " + to_string(commentLine) +
                    ", col " + to_string(commentCol) + ": Unterminated multi-line comment"
                );
            }
        }
        else {
            break;
        }
    }
}

Token Lexer::scanIdentifierOrKeyword() {
    int startLine = line;
    int startCol = column;
    string lexeme;

    while (!isAtEnd() && (isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        lexeme += advance();
    }

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return {it->second, lexeme, startLine, startCol};
    }
    return {ID, lexeme, startLine, startCol};
}

Token Lexer::scanNumber() {
    int startLine = line;
    int startCol = column;
    string lexeme;
    bool isFloat = false;

    while (!isAtEnd() && isdigit(static_cast<unsigned char>(peek()))) {
        lexeme += advance();
    }

    // Check for decimal fraction
    if (!isAtEnd() && peek() == '.' && isdigit(static_cast<unsigned char>(peek(1)))) {
        isFloat = true;
        lexeme += advance(); // consume '.'
        while (!isAtEnd() && isdigit(static_cast<unsigned char>(peek()))) {
            lexeme += advance();
        }
    }

    // Reject malformed numbers like 12.34.56 or 123abc
    if (!isAtEnd() && (peek() == '.' || isalpha(static_cast<unsigned char>(peek())) || peek() == '_')) {
        throw runtime_error(
            "Lexical Error at line " + to_string(startLine) +
            ", col " + to_string(startCol) + ": Malformed numeric literal '" + lexeme + peek() + "'"
        );
    }

    return {isFloat ? FLOAT_LITERAL : INT_LITERAL, lexeme, startLine, startCol};
}

Token Lexer::scanCharLiteral() {
    int startLine = line;
    int startCol = column;
    string lexeme;

    lexeme += advance(); // opening '\''

    if (isAtEnd() || peek() == '\n' || peek() == '\'') {
        throw runtime_error(
            "Lexical Error at line " + to_string(startLine) +
            ", col " + to_string(startCol) + ": Empty or unclosed character literal"
        );
    }

    // Support basic escape character (e.g., '\n', '\t', '\\', '\'')
    if (peek() == '\\') {
        lexeme += advance();
        if (isAtEnd()) {
            throw runtime_error(
                "Lexical Error at line " + to_string(startLine) +
                ", col " + to_string(startCol) + ": Unclosed escape sequence in char literal"
            );
        }
    }

    lexeme += advance(); // the character itself

    if (isAtEnd() || peek() != '\'') {
        throw runtime_error(
            "Lexical Error at line " + to_string(startLine) +
            ", col " + to_string(startCol) + ": Unclosed or multi-character literal"
        );
    }

    lexeme += advance(); // closing '\''
    return {CHAR_LITERAL, lexeme, startLine, startCol};
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        int startLine = line;
        int startCol = column;
        char c = peek();

        // Identifiers & Keywords
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(scanIdentifierOrKeyword());
            continue;
        }

        // Integer & Float Literals
        if (isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(scanNumber());
            continue;
        }

        // Character Literals
        if (c == '\'') {
            tokens.push_back(scanCharLiteral());
            continue;
        }

        // Two-character relational operators
        if (c == '=' && peek(1) == '=') {
            advance(); advance();
            tokens.push_back({OP_EQ, "==", startLine, startCol});
            continue;
        }
        if (c == '!' && peek(1) == '=') {
            advance(); advance();
            tokens.push_back({OP_NEQ, "!=", startLine, startCol});
            continue;
        }
        if (c == '<' && peek(1) == '=') {
            advance(); advance();
            tokens.push_back({OP_LTE, "<=", startLine, startCol});
            continue;
        }
        if (c == '>' && peek(1) == '=') {
            advance(); advance();
            tokens.push_back({OP_GTE, ">=", startLine, startCol});
            continue;
        }

        // Single-character operators & delimiters
        advance();
        switch (c) {
            case '+': tokens.push_back({OP_PLUS,  "+", startLine, startCol}); break;
            case '-': tokens.push_back({OP_MINUS, "-", startLine, startCol}); break;
            case '*': tokens.push_back({OP_MUL,   "*", startLine, startCol}); break;
            case '/': tokens.push_back({OP_DIV,   "/", startLine, startCol}); break;
            case '<': tokens.push_back({OP_LT,    "<", startLine, startCol}); break;
            case '>': tokens.push_back({OP_GT,    ">", startLine, startCol}); break;
            case '.': tokens.push_back({OP_DOT,   ".", startLine, startCol}); break;
            case '(': tokens.push_back({LPAREN,   "(", startLine, startCol}); break;
            case ')': tokens.push_back({RPAREN,   ")", startLine, startCol}); break;
            case '[': tokens.push_back({LBRACK,   "[", startLine, startCol}); break;
            case ']': tokens.push_back({RBRACK,   "]", startLine, startCol}); break;
            case ';': tokens.push_back({SEMI,     ";", startLine, startCol}); break;
            case ',': tokens.push_back({COMMA,    ",", startLine, startCol}); break;
            case '=':
                throw runtime_error(
                    "Lexical Error at line " + to_string(startLine) +
                    ", col " + to_string(startCol) +
                    ": Unexpected '=' (use 'medalo' for assignment or '==' for equality)"
                );
            default:
                throw runtime_error(
                    "Lexical Error at line " + to_string(startLine) +
                    ", col " + to_string(startCol) +
                    ": Unrecognized character '" + string(1, c) + "'"
                );
        }
    }

    tokens.push_back({TOKEN_EOF, "EOF", line, column});
    return tokens;
}

bool Lexer::writeTokensToFile(const vector<Token>& tokens, const string& outputPath) {
    ofstream out(outputPath);
    if (!out.is_open()) {
        return false;
    }

    for (const auto& tok : tokens) {
        out << left << setw(4) << tok.line << " | "
            << left << setw(4) << tok.column << " | "
            << left << setw(14) << tokenTypeToString(tok.type) << " | "
            << tok.lexeme << "\n";
    }

    return true;
}