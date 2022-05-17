/*  Lexer-Parser:test.cc
    To test the generating  of AST
*/

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<string> &ast);

int main(int argc, const char *argv[]){
    //format: compilerName mode input -o output 
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];
    
    yyin = fopen(input, "r");
    assert(yyin);

    unique_ptr<string> ast;
    auto err = yyparse(ast);
    assert(!err);

    cout << *ast << endl;
    return 0;
}