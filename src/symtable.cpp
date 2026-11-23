#include "../include/symtable.h"
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

// ==================== TypeUtils Implementation ====================

bool TypeUtils::isArrayType(const string& typeStr) {
    return typeStr.find('[') != string::npos && typeStr.back() == ']';
}

string TypeUtils::getBaseType(const string& typeStr) {
    size_t bracketPos = typeStr.find('[');
    if (bracketPos == string::npos) {
        return typeStr;
    }
    return typeStr.substr(0, bracketPos);
}

int TypeUtils::getArrayLength(const string& typeStr) {
    size_t openPos = typeStr.find('[');
    size_t closePos = typeStr.find(']');
    if (openPos == string::npos || closePos == string::npos || closePos <= openPos + 1) {
        return 0; // Unsized array parameter like "int32[]"
    }
    string lenStr = typeStr.substr(openPos + 1, closePos - openPos - 1);
    return stoi(lenStr);
}

bool TypeUtils::isStructType(const string& typeStr) {
    return !isArrayType(typeStr) && typeStr.rfind("struct ", 0) == 0;
}

string TypeUtils::getStructName(const string& typeStr) {
    if (!isStructType(typeStr)) {
        return "";
    }
    return typeStr.substr(7); // Strip "struct "
}

bool TypeUtils::isIntegerType(const string& typeStr) {
    return typeStr == "int16" || typeStr == "int32" ||
           typeStr == "uint16" || typeStr == "uint32" ||
           typeStr == "char";
}

bool TypeUtils::isNumericType(const string& typeStr) {
    return isIntegerType(typeStr) || typeStr == "float";
}

bool TypeUtils::areTypesCompatible(const string& targetType, const string& exprType) {
    if (targetType == exprType) {
        return true;
    }

    // Array parameter matching: passing "int32[20]" to parameter "int32[]"
    if (isArrayType(targetType) && isArrayType(exprType)) {
        if (getBaseType(targetType) == getBaseType(exprType) && getArrayLength(targetType) == 0) {
            return true;
        }
        return false;
    }

    // Scalar numeric compatibility (e.g., int16/uint16/char/int32/float)
    if (isNumericType(targetType) && isNumericType(exprType)) {
        return true;
    }

    return false;
}

// ==================== Scope Implementation ====================

Scope::Scope(int id, const string& name, Scope* parentScope, bool isFunc)
    : scopeId(id),
      scopeName(name),
      parent(parentScope),
      isFunctionScope(isFunc),
      nextLocalOffset(-4),
      nextParamOffset(8),
      nextFieldOffset(0) {
    // Inherit stack offset from enclosing function block so nested blocks don't collide
    if (!isFunc && parentScope != nullptr && parentScope->scopeId != 0) {
        nextLocalOffset = parentScope->nextLocalOffset;
    }
}

bool Scope::insert(const SymbolEntry& entry) {
    if (symbols.find(entry.name) != symbols.end()) {
        return false; // Duplicate symbol in the same scope
    }
    symbols[entry.name] = entry;
    insertionOrder.push_back(entry.name);
    return true;
}

SymbolEntry* Scope::lookupLocal(const string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &(it->second);
    }
    return nullptr;
}

SymbolEntry* Scope::lookup(const string& name) {
    Scope* curr = this;
    while (curr != nullptr) {
        SymbolEntry* found = curr->lookupLocal(name);
        if (found != nullptr) {
            return found;
        }
        curr = curr->parent;
    }
    return nullptr;
}

// ==================== SymbolTable Implementation ====================

SymbolTable::SymbolTable() {
    globalScope = new Scope(0, "global", nullptr, false);
    allScopes.push_back(globalScope);
    currentScope = globalScope;
}

SymbolTable::~SymbolTable() {
    for (Scope* s : allScopes) {
        delete s;
    }
}

int SymbolTable::alignTo4(int bytes) {
    if (bytes <= 0) return 4;
    return (bytes + 3) & ~3;
}

Scope* SymbolTable::getGlobalScope() const {
    return globalScope;
}

Scope* SymbolTable::getCurrentScope() const {
    return currentScope;
}

Scope* SymbolTable::enterScope(const string& name, bool isFunc) {
    int newId = static_cast<int>(allScopes.size());
    Scope* child = new Scope(newId, name, currentScope, isFunc);
    allScopes.push_back(child);
    currentScope = child;
    return child;
}

void SymbolTable::exitScope() {
    if (currentScope != nullptr && currentScope->parent != nullptr) {
        // Propagate stack offset growth back to enclosing block within the same function
        if (!currentScope->isFunctionScope && currentScope->parent->scopeId != 0) {
            currentScope->parent->nextLocalOffset = currentScope->nextLocalOffset;
        }
        currentScope = currentScope->parent;
    }
}

int SymbolTable::computeTypeSize(const string& typeStr, int line) const {
    if (TypeUtils::isArrayType(typeStr)) {
        string base = TypeUtils::getBaseType(typeStr);
        int len = TypeUtils::getArrayLength(typeStr);
        if (len == 0) {
            return 4; // Unsized array parameter decays to a 32-bit pointer
        }
        return computeTypeSize(base, line) * len;
    }

    if (typeStr == "char") {
        return 1;
    }
    if (typeStr == "int16" || typeStr == "uint16") {
        return 2;
    }
    if (typeStr == "int32" || typeStr == "uint32" || typeStr == "float") {
        return 4;
    }

    if (TypeUtils::isStructType(typeStr)) {
        string structName = TypeUtils::getStructName(typeStr);
        SymbolEntry* structSym = globalScope->lookupLocal(structName);
        if (structSym == nullptr || structSym->kind != SYM_STRUCT) {
            throw runtime_error(
                "Semantic Error at line " + to_string(line) +
                ": Unknown struct type '" + structName + "'"
            );
        }
        return structSym->size;
    }

    throw runtime_error(
        "Semantic Error at line " + to_string(line) +
        ": Unknown data type '" + typeStr + "'"
    );
}

bool SymbolTable::declareSymbol(SymbolEntry entry) {
    entry.scopeId = currentScope->scopeId;

    if (entry.kind == SYM_PARAM) {
        entry.size = computeTypeSize(entry.type, entry.line);
        entry.offset = currentScope->nextParamOffset;
        currentScope->nextParamOffset += alignTo4(entry.size);
    } else if (entry.kind == SYM_VAR) {
        entry.size = computeTypeSize(entry.type, entry.line);
        if (currentScope->scopeId == 0) {
            entry.offset = 0; // Global data segment variable
        } else {
            int slotBytes = alignTo4(entry.size);
            // Reserve full array/struct slot downward from EBP
            if (slotBytes > 4) {
                currentScope->nextLocalOffset -= (slotBytes - 4);
            }
            entry.offset = currentScope->nextLocalOffset;
            currentScope->nextLocalOffset -= 4;
        }
    } else if (entry.kind == SYM_FIELD) {
        entry.size = computeTypeSize(entry.type, entry.line);
        entry.offset = currentScope->nextFieldOffset;
        currentScope->nextFieldOffset += alignTo4(entry.size);
    } else if (entry.kind == SYM_FUNC) {
        entry.size = 0;
        entry.offset = 0;
    }
    // Note: SYM_STRUCT sets its size after its fields are inserted

    if (!currentScope->insert(entry)) {
        throw runtime_error(
            "Semantic Error at line " + to_string(entry.line) +
            ": Redeclaration of '" + entry.name + "' in scope '" + currentScope->scopeName + "'"
        );
    }
    return true;
}

SymbolEntry* SymbolTable::resolve(const string& name) const {
    return currentScope->lookup(name);
}

SymbolEntry* SymbolTable::resolveGlobal(const string& name) const {
    return globalScope->lookupLocal(name);
}

string SymbolTable::symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SYM_VAR:    return "SYM_VAR";
        case SYM_PARAM:  return "SYM_PARAM";
        case SYM_FUNC:   return "SYM_FUNC";
        case SYM_STRUCT: return "SYM_STRUCT";
        case SYM_FIELD:  return "SYM_FIELD";
        default:         return "SYM_UNKNOWN";
    }
}

bool SymbolTable::writeSymFile(const string& outputPath) const {
    ofstream out(outputPath);
    if (!out.is_open()) {
        return false;
    }

    out << left << setw(18) << "SCOPE" << " | "
        << left << setw(16) << "NAME" << " | "
        << left << setw(10) << "KIND" << " | "
        << left << setw(15) << "TYPE" << " | "
        << left << setw(6)  << "SIZE" << " | "
        << "OFFSET\n";
    out << string(80, '-') << "\n";

    for (const Scope* scope : allScopes) {
        string scopeLabel = to_string(scope->scopeId) + " (" + scope->scopeName + ")";
        for (const string& symName : scope->insertionOrder) {
            const SymbolEntry& sym = scope->symbols.at(symName);
            string offsetStr = (sym.kind == SYM_FUNC || sym.kind == SYM_STRUCT)
                ? "-"
                : ((sym.offset > 0 ? "+" : "") + to_string(sym.offset));

            out << left << setw(18) << scopeLabel << " | "
                << left << setw(16) << sym.name << " | "
                << left << setw(10) << symbolKindToString(sym.kind) << " | "
                << left << setw(15) << sym.type << " | "
                << left << setw(6)  << sym.size << " | "
                << offsetStr << "\n";
        }
    }

    return true;
}