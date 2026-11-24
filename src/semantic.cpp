#include "../include/semantic.h"
#include <stdexcept>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer()
    : currentFunctionName(""), currentFunctionReturnType(""), loopDepth(0) {}

void SemanticAnalyzer::analyzeProgram(ASTNode* root) {
    if (root == nullptr || root->kind != NODE_PROGRAM) {
        throw runtime_error("Semantic Error: Invalid or empty AST root");
    }

    for (ASTNode* child : root->children) {
        if (child->kind == NODE_STRUCT_DECL) {
            analyzeStructDecl(child);
        } else if (child->kind == NODE_VAR_DECL) {
            analyzeVarDecl(child);
        } else if (child->kind == NODE_FUNC_DECL) {
            analyzeFuncDecl(child);
        } else {
            throw runtime_error(
                "Semantic Error at line " + to_string(child->line) +
                ": Unexpected construct at global scope"
            );
        }
    }

    // Verify program entry point 'main' exists
    SymbolEntry* mainSym = symTable.resolveGlobal("main");
    if (mainSym == nullptr || mainSym->kind != SYM_FUNC) {
        throw runtime_error(
            "Semantic Error: Program is missing entry point function 'main'"
        );
    }
}

void SemanticAnalyzer::analyzeStructDecl(ASTNode* node) {
    string structName = node->value;

    // Register struct name in global scope first
    SymbolEntry structEntry;
    structEntry.name = structName;
    structEntry.kind = SYM_STRUCT;
    structEntry.type = "struct";
    structEntry.size = 0;
    structEntry.offset = 0;
    structEntry.line = node->line;
    symTable.declareSymbol(structEntry);

    SymbolEntry* storedStruct = symTable.resolveGlobal(structName);

    // Enter struct scope to compute member sizes and field offsets (0, 4, 8...)
    Scope* structScope = symTable.enterScope("struct:" + structName, false);

    for (ASTNode* fieldNode : node->children) {
        if (fieldNode->kind != NODE_VAR_DECL) {
            throw runtime_error(
                "Semantic Error at line " + to_string(fieldNode->line) +
                ": Only field declarations are allowed inside struct '" + structName + "'"
            );
        }

        // Prevent direct self-containment (infinite size)
        if (TypeUtils::getBaseType(fieldNode->dataType) == "struct " + structName) {
            throw runtime_error(
                "Semantic Error at line " + to_string(fieldNode->line) +
                ": Struct '" + structName + "' cannot contain an instance of itself"
            );
        }

        SymbolEntry fieldEntry;
        fieldEntry.name = fieldNode->value;
        fieldEntry.kind = SYM_FIELD;
        fieldEntry.type = fieldNode->dataType;
        fieldEntry.line = fieldNode->line;
        symTable.declareSymbol(fieldEntry);

        SymbolEntry* insertedField = structScope->lookupLocal(fieldEntry.name);
        if (insertedField != nullptr && storedStruct != nullptr) {
            storedStruct->fields[fieldEntry.name] = *insertedField;
        }
    }

    int totalBytes = structScope->nextFieldOffset;
    symTable.exitScope();

    if (storedStruct != nullptr) {
        storedStruct->size = (totalBytes > 0) ? totalBytes : 4;
    }
}

void SemanticAnalyzer::analyzeFuncDecl(ASTNode* node) {
    string funcName = node->value;
    string retType = node->dataType;

    // Validate return type exists (also catches unknown struct return types)
    symTable.computeTypeSize(retType, node->line);

    // Collect parameter types before entering function scope (enables recursion)
    vector<string> paramTypes;
    ASTNode* bodyBlock = nullptr;

    for (ASTNode* child : node->children) {
        if (child->kind == NODE_PARAM) {
            symTable.computeTypeSize(child->dataType, child->line);
            paramTypes.push_back(child->dataType);
        } else if (child->kind == NODE_BLOCK) {
            bodyBlock = child;
        }
    }

    SymbolEntry funcEntry;
    funcEntry.name = funcName;
    funcEntry.kind = SYM_FUNC;
    funcEntry.type = retType;
    funcEntry.line = node->line;
    funcEntry.paramTypes = paramTypes;
    symTable.declareSymbol(funcEntry);

    // Enter function scope (params at +8, +12...; locals at -4, -8...)
    currentFunctionName = funcName;
    currentFunctionReturnType = retType;
    symTable.enterScope(funcName, true);

    for (ASTNode* child : node->children) {
        if (child->kind == NODE_PARAM) {
            SymbolEntry paramEntry;
            paramEntry.name = child->value;
            paramEntry.kind = SYM_PARAM;
            paramEntry.type = child->dataType;
            paramEntry.line = child->line;
            symTable.declareSymbol(paramEntry);
        }
    }

    if (bodyBlock != nullptr) {
        // Function's outer shro..khatam shares the function scope with parameters
        analyzeBlock(bodyBlock, false);
    }

    symTable.exitScope();
    currentFunctionName = "";
    currentFunctionReturnType = "";
}

void SemanticAnalyzer::analyzeVarDecl(ASTNode* node) {
    if (TypeUtils::isArrayType(node->dataType) && TypeUtils::getArrayLength(node->dataType) <= 0) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Array variable '" + node->value + "' must specify a positive constant size"
        );
    }

    SymbolEntry varEntry;
    varEntry.name = node->value;
    varEntry.kind = SYM_VAR;
    varEntry.type = node->dataType;
    varEntry.line = node->line;
    symTable.declareSymbol(varEntry);
}

void SemanticAnalyzer::analyzeBlock(ASTNode* blockNode, bool createNewScope) {
    if (createNewScope) {
        symTable.enterScope("block", false);
    }

    for (ASTNode* child : blockNode->children) {
        analyzeStmt(child);
    }

    if (createNewScope) {
        symTable.exitScope();
    }
}

void SemanticAnalyzer::analyzeStmt(ASTNode* stmt) {
    switch (stmt->kind) {
        case NODE_VAR_DECL:
            analyzeVarDecl(stmt);
            break;
        case NODE_BLOCK:
            analyzeBlock(stmt, true);
            break;
        case NODE_ASSIGN:
            analyzeAssignStmt(stmt);
            break;
        case NODE_IF:
            analyzeIfStmt(stmt);
            break;
        case NODE_WHILE:
            analyzeWhileStmt(stmt);
            break;
        case NODE_BREAK:
            analyzeBreakStmt(stmt);
            break;
        case NODE_RETURN:
            analyzeReturnStmt(stmt);
            break;
        case NODE_FUNC_CALL:
            analyzeFuncCall(stmt);
            break;
        default:
            throw runtime_error(
                "Semantic Error at line " + to_string(stmt->line) +
                ": Unexpected AST statement node"
            );
    }
}

void SemanticAnalyzer::analyzeAssignStmt(ASTNode* node) {
    ASTNode* lhsNode = node->children[0];
    ASTNode* rhsNode = node->children[1];

    string lhsType = analyzeLValue(lhsNode);
    string rhsType = analyzeExpr(rhsNode);

    if (TypeUtils::isArrayType(lhsType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Cannot assign directly to entire array '" + lhsNode->value + "' (use an index '[i]')"
        );
    }

    if (!TypeUtils::areTypesCompatible(lhsType, rhsType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Type mismatch in 'medalo' assignment — cannot assign '" + rhsType +
            "' to '" + lhsType + "'"
        );
    }

    node->dataType = lhsType;
}

void SemanticAnalyzer::analyzeIfStmt(ASTNode* node) {
    string condType = analyzeExpr(node->children[0]);
    if (!TypeUtils::isNumericType(condType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": 'agar' condition must be a numeric/scalar expression (found '" + condType + "')"
        );
    }

    analyzeBlock(node->children[1], true);

    if (node->children.size() > 2) {
        analyzeBlock(node->children[2], true);
    }
}

void SemanticAnalyzer::analyzeWhileStmt(ASTNode* node) {
    string condType = analyzeExpr(node->children[0]);
    if (!TypeUtils::isNumericType(condType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": 'jabtak' condition must be a numeric/scalar expression (found '" + condType + "')"
        );
    }

    loopDepth++;
    analyzeBlock(node->children[1], true);
    loopDepth--;
}

void SemanticAnalyzer::analyzeBreakStmt(ASTNode* node) {
    if (loopDepth <= 0) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": 'nikal' statement is only valid inside a 'jabtak' loop"
        );
    }
}

void SemanticAnalyzer::analyzeReturnStmt(ASTNode* node) {
    if (currentFunctionName.empty()) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": 'return' statement outside of any function"
        );
    }

    if (node->children.empty()) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Function '" + currentFunctionName + "' must return a value of type '" +
            currentFunctionReturnType + "'"
        );
    }

    string retExprType = analyzeExpr(node->children[0]);
    if (!TypeUtils::areTypesCompatible(currentFunctionReturnType, retExprType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Return type mismatch in function '" + currentFunctionName +
            "' — expected '" + currentFunctionReturnType + "', found '" + retExprType + "'"
        );
    }

    node->dataType = retExprType;
}

string SemanticAnalyzer::analyzeExpr(ASTNode* expr) {
    switch (expr->kind) {
        case NODE_LITERAL:
            return expr->dataType;
        case NODE_LVALUE:
            return analyzeLValue(expr);
        case NODE_FUNC_CALL:
            return analyzeFuncCall(expr);
        case NODE_UNARY_EXPR:
            return analyzeUnaryExpr(expr);
        case NODE_BINARY_EXPR:
            return analyzeBinaryExpr(expr);
        default:
            throw runtime_error(
                "Semantic Error at line " + to_string(expr->line) +
                ": Invalid expression node in AST"
            );
    }
}

string SemanticAnalyzer::analyzeUnaryExpr(ASTNode* node) {
    string operandType = analyzeExpr(node->children[0]);
    if (!TypeUtils::isNumericType(operandType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Unary operator '" + node->value + "' requires a numeric operand (found '" +
            operandType + "')"
        );
    }

    node->dataType = operandType;
    return operandType;
}

string SemanticAnalyzer::analyzeBinaryExpr(ASTNode* node) {
    string leftType = analyzeExpr(node->children[0]);
    string rightType = analyzeExpr(node->children[1]);
    string op = node->value;

    if (!TypeUtils::isNumericType(leftType) || !TypeUtils::isNumericType(rightType)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(node->line) +
            ": Operator '" + op + "' cannot be applied to types '" +
            leftType + "' and '" + rightType + "'"
        );
    }

    // Relational & equality operators produce an integer boolean (0 or 1)
    if (op == "<" || op == "<=" || op == ">" || op == ">=" || op == "==" || op == "!=") {
        node->dataType = "int32";
        return "int32";
    }

    // Arithmetic operators (+, -, *, /): promote to float if either operand is float
    string resultType = (leftType == "float" || rightType == "float") ? "float" : "int32";
    node->dataType = resultType;
    return resultType;
}

string SemanticAnalyzer::analyzeLValue(ASTNode* lvalNode) {
    SymbolEntry* sym = symTable.resolve(lvalNode->value);
    if (sym == nullptr) {
        throw runtime_error(
            "Semantic Error at line " + to_string(lvalNode->line) +
            ": Undeclared identifier '" + lvalNode->value + "'"
        );
    }

    if (sym->kind == SYM_FUNC || sym->kind == SYM_STRUCT) {
        throw runtime_error(
            "Semantic Error at line " + to_string(lvalNode->line) +
            ": '" + lvalNode->value + "' is not a variable"
        );
    }

    string currentType = sym->type;

    for (ASTNode* modifier : lvalNode->children) {
        // Struct member access: child is NODE_LVALUE with ".memberName"
        if (modifier->kind == NODE_LVALUE && !modifier->value.empty() && modifier->value[0] == '.') {
            string memberName = modifier->value.substr(1);
            if (!TypeUtils::isStructType(currentType)) {
                throw runtime_error(
                    "Semantic Error at line " + to_string(modifier->line) +
                    ": Member access '." + memberName + "' requires a struct (found '" +
                    currentType + "')"
                );
            }

            string structName = TypeUtils::getStructName(currentType);
            SymbolEntry* structSym = symTable.resolveGlobal(structName);
            if (structSym == nullptr || structSym->fields.find(memberName) == structSym->fields.end()) {
                throw runtime_error(
                    "Semantic Error at line " + to_string(modifier->line) +
                    ": Struct '" + structName + "' has no member named '" + memberName + "'"
                );
            }

            currentType = structSym->fields[memberName].type;
            modifier->dataType = currentType;
        }
        // Array subscript access: child is an index expression
        else {
            if (!TypeUtils::isArrayType(currentType)) {
                throw runtime_error(
                    "Semantic Error at line " + to_string(modifier->line) +
                    ": Subscript '[...]' cannot be applied to non-array type '" + currentType + "'"
                );
            }

            string indexType = analyzeExpr(modifier);
            if (!TypeUtils::isIntegerType(indexType)) {
                throw runtime_error(
                    "Semantic Error at line " + to_string(modifier->line) +
                    ": Array index must be an integer type (found '" + indexType + "')"
                );
            }

            currentType = TypeUtils::getBaseType(currentType);
        }
    }

    lvalNode->dataType = currentType;
    return currentType;
}

string SemanticAnalyzer::analyzeFuncCall(ASTNode* callNode) {
    SymbolEntry* funcSym = symTable.resolveGlobal(callNode->value);
    if (funcSym == nullptr || funcSym->kind != SYM_FUNC) {
        throw runtime_error(
            "Semantic Error at line " + to_string(callNode->line) +
            ": Call to undeclared function '" + callNode->value + "'"
        );
    }

    if (callNode->children.size() != funcSym->paramTypes.size()) {
        throw runtime_error(
            "Semantic Error at line " + to_string(callNode->line) +
            ": Function '" + callNode->value + "' expects " +
            to_string(funcSym->paramTypes.size()) + " argument(s), but got " +
            to_string(callNode->children.size())
        );
    }

    for (size_t i = 0; i < callNode->children.size(); i++) {
        string argType = analyzeExpr(callNode->children[i]);
        string expectedType = funcSym->paramTypes[i];

        if (!TypeUtils::areTypesCompatible(expectedType, argType)) {
            throw runtime_error(
                "Semantic Error at line " + to_string(callNode->line) +
                ": Argument " + to_string(i + 1) + " of function '" + callNode->value +
                "' expects '" + expectedType + "', but got '" + argType + "'"
            );
        }
    }

    callNode->dataType = funcSym->type;
    return funcSym->type;
}

bool SemanticAnalyzer::writeSymbolTable(const string& outputPath) const {
    return symTable.writeSymFile(outputPath);
}