#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

enum SymbolKind {
    SYM_VAR,
    SYM_PARAM,
    SYM_FUNC,
    SYM_STRUCT,
    SYM_FIELD
};

struct SymbolEntry {
    string name;                                   // Identifier name
    SymbolKind kind;                               // Variable, param, function, struct, or field
    string type;                                   // e.g., "int32", "int32[20]", "struct Point"
    int size;                                      // Total byte size
    int offset;                                    // Stack offset (-4, +8) or struct field offset (0, 4)
    int scopeId;                                   // Owning scope ID
    int line;                                      // Declaration line number
    vector<string> paramTypes;                     // Ordered parameter types (for SYM_FUNC)
    unordered_map<string, SymbolEntry> fields;     // Member symbols (for SYM_STRUCT)
};

class TypeUtils {
public:
    static bool isArrayType(const string& typeStr);
    static string getBaseType(const string& typeStr);
    static int getArrayLength(const string& typeStr);

    static bool isStructType(const string& typeStr);
    static string getStructName(const string& typeStr);

    static bool isIntegerType(const string& typeStr);
    static bool isNumericType(const string& typeStr);
    static bool areTypesCompatible(const string& targetType, const string& exprType);
};

class Scope {
public:
    int scopeId;
    string scopeName;
    Scope* parent;
    bool isFunctionScope;

    unordered_map<string, SymbolEntry> symbols;
    vector<string> insertionOrder;

    int nextLocalOffset;   // Starts at -4, grows downward (-4, -8, -12...)
    int nextParamOffset;   // Starts at +8, grows upward (+8, +12, +16...)
    int nextFieldOffset;   // Starts at 0, grows upward (0, 4, 8...)

    Scope(int id, const string& name, Scope* parentScope = nullptr, bool isFunc = false);

    bool insert(const SymbolEntry& entry);
    SymbolEntry* lookupLocal(const string& name);
    SymbolEntry* lookup(const string& name);
};

class SymbolTable {
private:
    vector<Scope*> allScopes;
    Scope* globalScope;
    Scope* currentScope;

    static int alignTo4(int bytes);

public:
    SymbolTable();
    ~SymbolTable();

    Scope* getGlobalScope() const;
    Scope* getCurrentScope() const;

    Scope* enterScope(const string& name, bool isFunc = false);
    void exitScope();

    int computeTypeSize(const string& typeStr, int line) const;
    bool declareSymbol(SymbolEntry entry);
    SymbolEntry* resolve(const string& name) const;
    SymbolEntry* resolveGlobal(const string& name) const;

    static string symbolKindToString(SymbolKind kind);
    bool writeSymFile(const string& outputPath) const;
};

#endif // SYMTABLE_H