#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Parser {
private:
    vector<Token> tokens;
    size_t current;

    // Token navigation & matching helpers
    const Token& peek(size_t offset = 0) const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(const vector<TokenType>& types);
    const Token& advance();
    const Token& expect(TokenType type, const string& errorMessage);

    // Grammar helpers
    bool isTypeKeyword(TokenType type) const;
    bool isTypeStart() const;
    string parseType();

    // Top-level & Block productions
    ASTNode* parseTopLevelDecl();
    ASTNode* parseStructDecl();
    ASTNode* parseFuncDecl();
    vector<ASTNode*> parseParamList();
    ASTNode* parseParam();
    ASTNode* parseVarDecl();
    ASTNode* parseBlock();

    // Statement productions
    ASTNode* parseStmt();
    ASTNode* parseIfStmt();
    ASTNode* parseWhileStmt();
    ASTNode* parseBreakStmt();
    ASTNode* parseReturnStmt();
    ASTNode* parseAssignOrCallStmt();
    ASTNode* parseLValue();

    // Expression productions (lowest to highest precedence)
    ASTNode* parseExpr();
    ASTNode* parseEqualityExpr();
    ASTNode* parseRelationalExpr();
    ASTNode* parseAdditiveExpr();
    ASTNode* parseMultiplicativeExpr();
    ASTNode* parseUnaryExpr();
    ASTNode* parsePrimaryExpr();
    ASTNode* parseFuncCall();

    // Recursive AST file printer
    static void printASTNode(const ASTNode* node, ofstream& out, const string& prefix, bool isLast);

public:
    explicit Parser(const vector<Token>& tokenList);

    ASTNode* parseProgram();
    static bool writeASTToFile(const ASTNode* root, const string& outputPath);
};

#endif // PARSER_H