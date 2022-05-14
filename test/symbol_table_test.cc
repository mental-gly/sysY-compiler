#include "logging.h"
#include "AST/SymbolTable.h"
#include <string>

int main() {
    SymbolTable<std::string> table;
    table.insertSymbol("a", "int");
    table.CreateScope();
    table.insertSymbol("a", "double");
    table.insertSymbol("b", "int");
    CHECK_EQ(table.lookup("a"), "double");
    table.LeaveScope();
    CHECK_EQ(table.lookup("a"), "int");
    // would cause error
    table.lookup("b");
}