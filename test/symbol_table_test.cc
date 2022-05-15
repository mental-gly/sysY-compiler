#include "logging.h"
#include "AST/SymbolTable.h"
#include <string>

int main() {
    SymbolTable<int> table;
    table.insertSymbol("a", 1);
    table.CreateScope();
    table.insertSymbol("a", 2);
    table.insertSymbol("b", 1);
    CHECK_EQ(table.lookup("a"), 2);
    table.LeaveScope();
    CHECK_EQ(table.lookup("a"), 1);
    // would cause error
    // table.lookup("b");
    table.LeaveScope();
}