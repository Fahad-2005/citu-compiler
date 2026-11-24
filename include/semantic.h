#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtable.h"
#include <string>

using namespace std;

class SemanticAnalyzer {
private:
    SymbolTable symTable;
    string currentFunctionName;
    string currentFunctionReturnType;
    int loopDepth;

    // Top-level & declaration visitors
    void analyzeStructDecl(ASTNode* node);
    void analyzeFuncDecl(ASTNode* node);
    void analyzeVarDecl(ASTNode* node);
    void analyzeBlock(ASTNode* blockNode, bool createNewScope = true);

    // Statement visitors
    void analyzeStmt(ASTNode* stmt);
    void analyzeAssignStmt(ASTNode* node);
    void analyzeIfStmt(ASTNode* node);
    void analyzeWhileStmt(ASTNode* node);
    void analyzeBreakStmt(ASTNode* node);
    void analyzeReturnStmt(ASTNode* node);

    // Expression & LValue type checkers (return resolved dataType)
    string analyzeExpr(ASTNode* expr);
    string analyzeBinaryExpr(ASTNode* node);
    string analyzeUnaryExpr(ASTNode* node);
    string analyzeLValue(ASTNode* lvalNode);
    string analyzeFuncCall(ASTNode* callNode);

public:
    SemanticAnalyzer();

    void analyzeProgram(ASTNode* root);
    bool writeSymbolTable(const string& outputPath) const;
};

#endif // SEMANTIC_H