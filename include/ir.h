#ifndef IR_H
#define IR_H

#include <string>
#include <vector>

using namespace std;

enum TACOp {
    IR_FUNC_BEGIN,   // func <name>:
    IR_FUNC_END,     // end func <name>
    IR_ASSIGN,       // result = arg1
    IR_ADD,          // result = arg1 + arg2
    IR_SUB,          // result = arg1 - arg2
    IR_MUL,          // result = arg1 * arg2
    IR_DIV,          // result = arg1 / arg2
    IR_NEG,          // result = -arg1
    IR_LT,           // result = arg1 < arg2
    IR_LTE,          // result = arg1 <= arg2
    IR_GT,           // result = arg1 > arg2
    IR_GTE,          // result = arg1 >= arg2
    IR_EQ,           // result = arg1 == arg2
    IR_NEQ,          // result = arg1 != arg2
    IR_LABEL,        // <result>:
    IR_GOTO,         // goto <result>
    IR_IF_FALSE,     // ifFalse <arg1> goto <result>
    IR_ARRAY_LOAD,   // result = arg1[arg2]  (arg2 is byte offset)
    IR_ARRAY_STORE,  // result[arg1] = arg2  (arg1 is byte offset)
    IR_PARAM,        // param <arg1>
    IR_CALL,         // result = call <arg1>, <arg2> (arg2 is arg count)
    IR_RETURN        // return <arg1>
};

struct TACInstr {
    TACOp op;
    string result;
    string arg1;
    string arg2;

    TACInstr(TACOp operation,
             const string& res = "",
             const string& a1 = "",
             const string& a2 = "")
        : op(operation), result(res), arg1(a1), arg2(a2) {}

    string toString() const;
};

class IRModule {
private:
    vector<TACInstr> instructions;
    int tempCounter;
    int labelCounter;

public:
    IRModule();

    string newTemp();
    string newLabel();

    void emit(TACOp op, const string& result = "", const string& arg1 = "", const string& arg2 = "");
    void emit(const TACInstr& instr);

    const vector<TACInstr>& getInstructions() const;
    size_t size() const;

    bool writeIRToFile(const string& outputPath) const;
};

#endif // IR_H