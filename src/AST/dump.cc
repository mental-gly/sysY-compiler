//===-- Debug dumping functions --==//
// We use a static indent size to control
// the output
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

static int dump_indent = 0;
#define DUMP_WITH_IDENT(X, Y)              \
    for (int i = 0; i < X; ++i)            \
      errs() << "  ";                      \
    errs() << #Y << ": "
#define DUMP_NEWLINE(X) errs() << X << "\n"

//===-- Statements dumps --===//

void CompoundStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, CompundStmt);
    DUMP_NEWLINE("");
    dump_indent++;
    for (auto &stmt : Stmts) {
        stmt->dump();
    }
    dump_indent--;
}

void DeclStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, DeclStmt);
    DUMP_NEWLINE("");
    dump_indent++;
    for (auto &decl : Decls) {
        decl->dump();
    }
    dump_indent--;
}

void IfStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, IfStmt);
    dump_indent++;
    DUMP_NEWLINE("");
    DUMP_WITH_IDENT(dump_indent, Cond);
    DUMP_NEWLINE("");
    dump_indent++;
    Cond->dump();
    dump_indent--;
    if (Then != nullptr) {
        DUMP_WITH_IDENT(dump_indent, Then);
        DUMP_NEWLINE("");
        dump_indent++;
        Then->dump();
        dump_indent--;
    }
    if (Else != nullptr) {
        DUMP_WITH_IDENT(dump_indent, Else);
        DUMP_NEWLINE("");
        dump_indent++;
        Else->dump();
        dump_indent--;
    }
    dump_indent--;
}

void ReturnStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, ReturnStmt);
    dump_indent++;
    DUMP_NEWLINE("");
    RetExpr->dump();
    dump_indent--;
}

void CallStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, CallStmt);
    DUMP_NEWLINE(Callee);
    dump_indent++;
    for (auto &args : Args) {
        args->dump();
    }
}

void ArraySubscriptStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, ArraySubscript);
    dump_indent++;
    DUMP_NEWLINE("");
    Base->dump();
    DUMP_NEWLINE("");
    Idx->dump();
    dump_indent--;
}

void DeclRefStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, DeclRef);
    DUMP_NEWLINE(Symbol);
}

void ::BinaryOperatorStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, Binary);
    DUMP_NEWLINE((uint32_t)(Opcode));
    dump_indent++;
    SubExprs[0]->dump();
    SubExprs[1]->dump();
    dump_indent--;
}

void ::IntegerLiteral::dump() {
    DUMP_WITH_IDENT(dump_indent, Integer);
    DUMP_NEWLINE(Value.getSExtValue());
}

void ::FloatingLiteral::dump() {
    DUMP_WITH_IDENT(dump_indent, Float);
    DUMP_NEWLINE(Value.convertToDouble());
}

void ::StringLiteral::dump() {
    DUMP_WITH_IDENT(dump_indent, String);
    DUMP_NEWLINE(Literal);
}

void WhileStmt::dump() {

}


//===-- Declaration dumps --===//

void CompileUnitDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, CompileUnit);
    DUMP_NEWLINE(Name);
    dump_indent++;
    for (auto &decl : Decls) {
        decl->dump();
    }
    dump_indent--;
}

void FunctionDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, Function);
    DUMP_NEWLINE(Name);
    dump_indent++;
    for (auto &param : Params) {
        param->dump();
    }
    if (hasBody()) Body->dump();
    dump_indent--;
}

void VarDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, VarDecl);
    DUMP_NEWLINE(Name);
}

void ParamDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, VarDecl);
    DUMP_NEWLINE(Name);
}
