#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Lexer {
private:
    string source;
    size_t pos;
    int line;
    int column;
    unordered_map<string, TokenType> keywords;

    char peek(size_t offset = 0) const;
    char advance();
    bool isAtEnd() const;

    void skipWhitespaceAndComments();
    Token scanIdentifierOrKeyword();
    Token scanNumber();
    Token scanCharLiteral();

public:
    explicit Lexer(const string& src);

    vector<Token> tokenize();
    static bool writeTokensToFile(const vector<Token>& tokens, const string& outputPath);
};

#endif // LEXER_H