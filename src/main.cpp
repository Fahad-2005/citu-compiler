#include "../include/lexer.h"
#include "../include/parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MAKE_DIR(dir) mkdir(dir, 0755)
#endif

string extractBaseName(const string& filePath) {
    size_t lastSlash = filePath.find_last_of("/\\");
    string fileName = (lastSlash == string::npos)
        ? filePath
        : filePath.substr(lastSlash + 1);

    size_t lastDot = fileName.find_last_of('.');
    if (lastDot != string::npos) {
        return fileName.substr(0, lastDot);
    }
    return fileName;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: citu <source_file.citu>\n";
        return 1;
    }

    string inputPath = argv[1];
    ifstream inFile(inputPath);
    if (!inFile.is_open()) {
        cerr << "Error: Could not open source file '" << inputPath << "'\n";
        return 1;
    }

    stringstream buffer;
    buffer << inFile.rdbuf();
    string sourceCode = buffer.str();
    inFile.close();

    MAKE_DIR("build");
    string baseName = extractBaseName(inputPath);
    string tokOutputPath = "build/" + baseName + ".tok";
    string astOutputPath = "build/" + baseName + ".ast";

    ASTNode* astRoot = nullptr;

    try {
        // Stage 1: Lexical Analysis
        Lexer lexer(sourceCode);
        vector<Token> tokens = lexer.tokenize();

        if (!Lexer::writeTokensToFile(tokens, tokOutputPath)) {
            cerr << "Error: Failed to write token file '" << tokOutputPath << "'\n";
            return 1;
        }
        cout << "[Lexer]  Tokenized " << tokens.size() << " tokens -> " << tokOutputPath << "\n";

        // Stage 2: Syntax Analysis (Parsing)
        Parser parser(tokens);
        astRoot = parser.parseProgram();

        if (!Parser::writeASTToFile(astRoot, astOutputPath)) {
            delete astRoot;
            cerr << "Error: Failed to write AST file '" << astOutputPath << "'\n";
            return 1;
        }
        cout << "[Parser] Generated AST -> " << astOutputPath << "\n";

        delete astRoot;
    } catch (const exception& ex) {
        delete astRoot;
        cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}