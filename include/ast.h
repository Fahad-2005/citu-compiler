#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

using namespace std;

enum NodeKind {
    NODE_PROGRAM,
    NODE_FUNC_DECL,
    NODE_STRUCT_DECL,
    NODE_VAR_DECL,
    NODE_PARAM,
    NODE_BLOCK,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_BREAK,
    NODE_RETURN,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_FUNC_CALL,
    NODE_LVALUE,
    NODE_LITERAL
};

struct ASTNode {
    NodeKind kind;              // Construct category
    string value;               // Identifier, operator, or literal text
    string dataType;            // Type annotation (e.g., "int32", "int32[20]")
    int line;                   // Source line number
    vector<ASTNode*> children;  // Child subtrees

    ASTNode(NodeKind k, const string& val = "", const string& type = "", int ln = 0)
        : kind(k), value(val), dataType(type), line(ln) {}

    ~ASTNode() {
        for (ASTNode* child : children) {
            delete child;
        }
    }

    void addChild(ASTNode* child) {
        if (child != nullptr) {
            children.push_back(child);
        }
    }
};

string nodeKindToString(NodeKind kind);

#endif // AST_H