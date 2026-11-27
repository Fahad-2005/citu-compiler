#include "../include/ir.h"
#include <fstream>

using namespace std;

string TACInstr::toString() const {
    switch (op) {
        case IR_FUNC_BEGIN:
            return "func " + result + ":";
        case IR_FUNC_END:
            return "end func " + result + "\n";
        case IR_ASSIGN:
            return "    " + result + " = " + arg1;
        case IR_ADD:
            return "    " + result + " = " + arg1 + " + " + arg2;
        case IR_SUB:
            return "    " + result + " = " + arg1 + " - " + arg2;
        case IR_MUL:
            return "    " + result + " = " + arg1 + " * " + arg2;
        case IR_DIV:
            return "    " + result + " = " + arg1 + " / " + arg2;
        case IR_NEG:
            return "    " + result + " = -" + arg1;
        case IR_LT:
            return "    " + result + " = " + arg1 + " < " + arg2;
        case IR_LTE:
            return "    " + result + " = " + arg1 + " <= " + arg2;
        case IR_GT:
            return "    " + result + " = " + arg1 + " > " + arg2;
        case IR_GTE:
            return "    " + result + " = " + arg1 + " >= " + arg2;
        case IR_EQ:
            return "    " + result + " = " + arg1 + " == " + arg2;
        case IR_NEQ:
            return "    " + result + " = " + arg1 + " != " + arg2;
        case IR_LABEL:
            return result + ":";
        case IR_GOTO:
            return "    goto " + result;
        case IR_IF_FALSE:
            return "    ifFalse " + arg1 + " goto " + result;
        case IR_ARRAY_LOAD:
            return "    " + result + " = " + arg1 + "[" + arg2 + "]";
        case IR_ARRAY_STORE:
            return "    " + result + "[" + arg1 + "] = " + arg2;
        case IR_PARAM:
            return "    param " + arg1;
        case IR_CALL:
            if (result.empty()) {
                return "    call " + arg1 + ", " + arg2;
            }
            return "    " + result + " = call " + arg1 + ", " + arg2;
        case IR_RETURN:
            if (arg1.empty()) {
                return "    return";
            }
            return "    return " + arg1;
        default:
            return "    ; unknown_op";
    }
}

IRModule::IRModule() : tempCounter(0), labelCounter(0) {}

string IRModule::newTemp() {
    return "t" + to_string(tempCounter++);
}

string IRModule::newLabel() {
    return "L" + to_string(labelCounter++);
}

void IRModule::emit(TACOp op, const string& result, const string& arg1, const string& arg2) {
    instructions.emplace_back(op, result, arg1, arg2);
}

void IRModule::emit(const TACInstr& instr) {
    instructions.push_back(instr);
}

const vector<TACInstr>& IRModule::getInstructions() const {
    return instructions;
}

size_t IRModule::size() const {
    return instructions.size();
}

bool IRModule::writeIRToFile(const string& outputPath) const {
    ofstream out(outputPath);
    if (!out.is_open()) {
        return false;
    }

    out << "; Citu Three-Address Code (TAC) Intermediate Representation\n\n";
    for (const TACInstr& instr : instructions) {
        out << instr.toString() << "\n";
    }

    return true;
}