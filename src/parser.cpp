#include "../include/parser.h"
#include <stdexcept>

using namespace std;

string nodeKindToString(NodeKind kind) {
    switch (kind) {
        case NODE_PROGRAM:     return "Program";
        case NODE_FUNC_DECL:   return "FuncDecl";
        case NODE_STRUCT_DECL: return "StructDecl";
        case NODE_VAR_DECL:    return "VarDecl";
        case NODE_PARAM:       return "Param";
        case NODE_BLOCK:       return "Block";
        case NODE_ASSIGN:      return "AssignStmt";
        case NODE_IF:          return "IfStmt";
        case NODE_WHILE:       return "WhileStmt";
        case NODE_BREAK:       return "BreakStmt";
        case NODE_RETURN:      return "ReturnStmt";
        case NODE_BINARY_EXPR: return "BinaryExpr";
        case NODE_UNARY_EXPR:  return "UnaryExpr";
        case NODE_FUNC_CALL:   return "FuncCall";
        case NODE_LVALUE:      return "LValue";
        case NODE_LITERAL:     return "Literal";
        default:               return "UnknownNode";
    }
}

Parser::Parser(const vector<Token>& tokenList)
    : tokens(tokenList), current(0) {}

const Token& Parser::peek(size_t offset) const {
    if (current + offset >= tokens.size()) {
        return tokens.back(); // TOKEN_EOF
    }
    return tokens[current + offset];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TOKEN_EOF;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(const vector<TokenType>& types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

const Token& Parser::expect(TokenType type, const string& errorMessage) {
    if (check(type)) {
        return advance();
    }
    const Token& bad = peek();
    throw runtime_error(
        "Syntax Error at line " + to_string(bad.line) +
        ", col " + to_string(bad.column) + ": " + errorMessage +
        " (found '" + bad.lexeme + "')"
    );
}

bool Parser::isTypeKeyword(TokenType type) const {
    return type == KW_INT16 || type == KW_INT32 ||
           type == KW_UINT16 || type == KW_UINT32 ||
           type == KW_FLOAT || type == KW_CHAR;
}

bool Parser::isTypeStart() const {
    if (isTypeKeyword(peek().type)) {
        return true;
    }
    // Struct variable/parameter type: "struct StructName varName"
    if (peek().type == KW_STRUCT && peek(1).type == ID && peek(2).type == ID) {
        return true;
    }
    return false;
}

string Parser::parseType() {
    if (isTypeKeyword(peek().type)) {
        return advance().lexeme;
    }
    if (match({KW_STRUCT})) {
        Token structName = expect(ID, "Expected struct name after 'struct'");
        return "struct " + structName.lexeme;
    }
    const Token& bad = peek();
    throw runtime_error(
        "Syntax Error at line " + to_string(bad.line) +
        ", col " + to_string(bad.column) + ": Expected a valid type name"
    );
}

// Program -> TopLevelDecl* TOKEN_EOF
ASTNode* Parser::parseProgram() {
    ASTNode* program = new ASTNode(NODE_PROGRAM, "citu_program", "", 1);
    try {
        while (!isAtEnd()) {
            program->addChild(parseTopLevelDecl());
        }
        expect(TOKEN_EOF, "Expected end of file");
    } catch (...) {
        delete program;
        throw;
    }
    return program;
}

// TopLevelDecl -> StructDecl | FuncDecl | GlobalVarDecl
ASTNode* Parser::parseTopLevelDecl() {
    // Struct definition: struct ID shro ... khatam
    if (peek().type == KW_STRUCT && peek(1).type == ID && peek(2).type == KW_SHRO) {
        return parseStructDecl();
    }

    if (isTypeStart()) {
        // Look ahead past the type and identifier to see if '(' follows (Function vs Global Var)
        size_t idOffset = (peek().type == KW_STRUCT) ? 2 : 1;
        if (peek(idOffset).type == ID && peek(idOffset + 1).type == LPAREN) {
            return parseFuncDecl();
        }
        return parseVarDecl();
    }

    const Token& bad = peek();
    throw runtime_error(
        "Syntax Error at line " + to_string(bad.line) +
        ", col " + to_string(bad.column) +
        ": Expected function, struct, or variable declaration at top level (found '" + bad.lexeme + "')"
    );
}

// StructDecl -> struct ID shro VarDecl* khatam ;?
ASTNode* Parser::parseStructDecl() {
    Token kw = expect(KW_STRUCT, "Expected 'struct'");
    Token name = expect(ID, "Expected struct identifier");
    ASTNode* structNode = new ASTNode(NODE_STRUCT_DECL, name.lexeme, "struct", kw.line);

    expect(KW_SHRO, "Expected 'shro' to open struct body");
    while (!check(KW_KHATAM) && !isAtEnd()) {
        structNode->addChild(parseVarDecl());
    }
    expect(KW_KHATAM, "Expected 'khatam' to close struct body");
    match({SEMI}); // Optional trailing semicolon
    return structNode;
}

// FuncDecl -> Type ID ( ParamList? ) Block
ASTNode* Parser::parseFuncDecl() {
    int declLine = peek().line;
    string retType = parseType();
    Token funcName = expect(ID, "Expected function name");

    ASTNode* funcNode = new ASTNode(NODE_FUNC_DECL, funcName.lexeme, retType, declLine);

    expect(LPAREN, "Expected '(' after function name");
    if (!check(RPAREN)) {
        vector<ASTNode*> params = parseParamList();
        for (ASTNode* p : params) {
            funcNode->addChild(p);
        }
    }
    expect(RPAREN, "Expected ')' after parameter list");

    funcNode->addChild(parseBlock());
    return funcNode;
}

vector<ASTNode*> Parser::parseParamList() {
    vector<ASTNode*> params;
    params.push_back(parseParam());
    while (match({COMMA})) {
        params.push_back(parseParam());
    }
    return params;
}

// Param -> Type ID ( [ INT_LITERAL? ] )?
ASTNode* Parser::parseParam() {
    int paramLine = peek().line;
    string paramType = parseType();
    Token paramName = expect(ID, "Expected parameter name");

    if (match({LBRACK})) {
        string sizeStr = "";
        if (check(INT_LITERAL)) {
            sizeStr = advance().lexeme;
        }
        expect(RBRACK, "Expected ']' in array parameter");
        paramType += "[" + sizeStr + "]";
    }

    return new ASTNode(NODE_PARAM, paramName.lexeme, paramType, paramLine);
}

// VarDecl -> Type ID ( [ INT_LITERAL ] )? ;
ASTNode* Parser::parseVarDecl() {
    int declLine = peek().line;
    string varType = parseType();
    Token varName = expect(ID, "Expected variable identifier");

    if (match({LBRACK})) {
        Token sizeTok = expect(INT_LITERAL, "Expected constant integer size inside '[ ]'");
        expect(RBRACK, "Expected ']' after array size");
        varType += "[" + sizeTok.lexeme + "]";
    }

    expect(SEMI, "Expected ';' after variable declaration");
    return new ASTNode(NODE_VAR_DECL, varName.lexeme, varType, declLine);
}

// Block -> shro VarDecl* Stmt* khatam (Enforces C89 top-of-block declarations)
ASTNode* Parser::parseBlock() {
    Token openTok = expect(KW_SHRO, "Expected 'shro' to start block");
    ASTNode* blockNode = new ASTNode(NODE_BLOCK, "shro..khatam", "", openTok.line);

    // Phase 1: All local declarations must appear at the top of the block
    while (isTypeStart()) {
        blockNode->addChild(parseVarDecl());
    }

    // Phase 2: Executable statements
    while (!check(KW_KHATAM) && !isAtEnd()) {
        if (isTypeStart()) {
            const Token& bad = peek();
            throw runtime_error(
                "Syntax Error at line " + to_string(bad.line) +
                ", col " + to_string(bad.column) +
                ": C89 rule violation — variable declarations must appear at the top of the 'shro' block before any statements"
            );
        }
        blockNode->addChild(parseStmt());
    }

    expect(KW_KHATAM, "Expected 'khatam' to close block");
    return blockNode;
}

// Stmt -> Block | IfStmt | WhileStmt | BreakStmt | ReturnStmt | AssignOrCallStmt
ASTNode* Parser::parseStmt() {
    if (check(KW_SHRO))   return parseBlock();
    if (check(KW_AGAR))   return parseIfStmt();
    if (check(KW_JABTAK)) return parseWhileStmt();
    if (check(KW_NIKAL))  return parseBreakStmt();
    if (check(KW_RETURN)) return parseReturnStmt();
    if (check(ID))        return parseAssignOrCallStmt();

    const Token& bad = peek();
    throw runtime_error(
        "Syntax Error at line " + to_string(bad.line) +
        ", col " + to_string(bad.column) +
        ": Unexpected token '" + bad.lexeme + "' at start of statement"
    );
}

// IfStmt -> agar ( Expr ) Block ( magar Block )?
ASTNode* Parser::parseIfStmt() {
    Token kw = expect(KW_AGAR, "Expected 'agar'");
    ASTNode* ifNode = new ASTNode(NODE_IF, "agar", "", kw.line);

    expect(LPAREN, "Expected '(' after 'agar'");
    ifNode->addChild(parseExpr());
    expect(RPAREN, "Expected ')' after 'agar' condition");

    ifNode->addChild(parseBlock());

    if (match({KW_MAGAR})) {
        ifNode->addChild(parseBlock());
    }

    return ifNode;
}

// WhileStmt -> jabtak ( Expr ) Block
ASTNode* Parser::parseWhileStmt() {
    Token kw = expect(KW_JABTAK, "Expected 'jabtak'");
    ASTNode* whileNode = new ASTNode(NODE_WHILE, "jabtak", "", kw.line);

    expect(LPAREN, "Expected '(' after 'jabtak'");
    whileNode->addChild(parseExpr());
    expect(RPAREN, "Expected ')' after 'jabtak' condition");

    whileNode->addChild(parseBlock());
    return whileNode;
}

// BreakStmt -> nikal ;
ASTNode* Parser::parseBreakStmt() {
    Token kw = expect(KW_NIKAL, "Expected 'nikal'");
    expect(SEMI, "Expected ';' after 'nikal'");
    return new ASTNode(NODE_BREAK, "nikal", "", kw.line);
}

// ReturnStmt -> return Expr? ;
ASTNode* Parser::parseReturnStmt() {
    Token kw = expect(KW_RETURN, "Expected 'return'");
    ASTNode* retNode = new ASTNode(NODE_RETURN, "return", "", kw.line);

    if (!check(SEMI)) {
        retNode->addChild(parseExpr());
    }
    expect(SEMI, "Expected ';' after return statement");
    return retNode;
}

// AssignStmt -> LValue medalo Expr ;  OR  FuncCallStmt -> ID ( ArgList? ) ;
ASTNode* Parser::parseAssignOrCallStmt() {
    if (peek().type == ID && peek(1).type == LPAREN) {
        ASTNode* callNode = parseFuncCall();
        expect(SEMI, "Expected ';' after function call");
        return callNode;
    }

    ASTNode* target = parseLValue();
    Token op = expect(OP_MEDALO, "Expected 'medalo' in assignment statement");
    ASTNode* rhs = parseExpr();
    expect(SEMI, "Expected ';' after assignment statement");

    ASTNode* assignNode = new ASTNode(NODE_ASSIGN, "medalo", "", op.line);
    assignNode->addChild(target);
    assignNode->addChild(rhs);
    return assignNode;
}

// LValue -> ID ( [ Expr ] | . ID )*
ASTNode* Parser::parseLValue() {
    Token base = expect(ID, "Expected identifier");
    ASTNode* lvalNode = new ASTNode(NODE_LVALUE, base.lexeme, "", base.line);

    while (check(LBRACK) || check(OP_DOT)) {
        if (match({LBRACK})) {
            lvalNode->addChild(parseExpr());
            expect(RBRACK, "Expected ']' after array index expression");
        } else if (match({OP_DOT})) {
            Token member = expect(ID, "Expected struct member name after '.'");
            lvalNode->addChild(new ASTNode(NODE_LVALUE, "." + member.lexeme, "", member.line));
        }
    }
    return lvalNode;
}

ASTNode* Parser::parseExpr() {
    return parseEqualityExpr();
}

// EqualityExpr -> RelationalExpr ( ( == | != ) RelationalExpr )*
ASTNode* Parser::parseEqualityExpr() {
    ASTNode* left = parseRelationalExpr();
    while (match({OP_EQ, OP_NEQ})) {
        Token op = previous();
        ASTNode* right = parseRelationalExpr();
        ASTNode* bin = new ASTNode(NODE_BINARY_EXPR, op.lexeme, "", op.line);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// RelationalExpr -> AdditiveExpr ( ( < | <= | > | >= ) AdditiveExpr )*
ASTNode* Parser::parseRelationalExpr() {
    ASTNode* left = parseAdditiveExpr();
    while (match({OP_LT, OP_LTE, OP_GT, OP_GTE})) {
        Token op = previous();
        ASTNode* right = parseAdditiveExpr();
        ASTNode* bin = new ASTNode(NODE_BINARY_EXPR, op.lexeme, "", op.line);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// AdditiveExpr -> MultiplicativeExpr ( ( + | - ) MultiplicativeExpr )*
ASTNode* Parser::parseAdditiveExpr() {
    ASTNode* left = parseMultiplicativeExpr();
    while (match({OP_PLUS, OP_MINUS})) {
        Token op = previous();
        ASTNode* right = parseMultiplicativeExpr();
        ASTNode* bin = new ASTNode(NODE_BINARY_EXPR, op.lexeme, "", op.line);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// MultiplicativeExpr -> UnaryExpr ( ( * | / ) UnaryExpr )*
ASTNode* Parser::parseMultiplicativeExpr() {
    ASTNode* left = parseUnaryExpr();
    while (match({OP_MUL, OP_DIV})) {
        Token op = previous();
        ASTNode* right = parseUnaryExpr();
        ASTNode* bin = new ASTNode(NODE_BINARY_EXPR, op.lexeme, "", op.line);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// UnaryExpr -> ( - | + ) UnaryExpr | PrimaryExpr
ASTNode* Parser::parseUnaryExpr() {
    if (match({OP_MINUS, OP_PLUS})) {
        Token op = previous();
        ASTNode* unary = new ASTNode(NODE_UNARY_EXPR, op.lexeme, "", op.line);
        unary->addChild(parseUnaryExpr());
        return unary;
    }
    return parsePrimaryExpr();
}

// PrimaryExpr -> INT_LITERAL | FLOAT_LITERAL | CHAR_LITERAL | ( Expr ) | FuncCall | LValue
