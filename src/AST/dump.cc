//===-- Debug dumping functions --==//
// We use a static indent size to control
// the output
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#if !defined(NDEBUG)
static int dump_indent = 0;
SmallVector<int, 10> Indent;

#define DUMP_WITH_IDENT(X, Y)                   \
    for (int i = 0, bar = 0; i < X; ++i)  {     \
      if (bar < Indent.size()                   \
        && i == Indent[bar]) {                  \
            errs() << "| ";                     \
            bar++;                              \
        }                                       \
        else errs() << "  ";                    \
      }                                         \
    errs() << #Y << ": "
#define DUMP_NEWLINE(X) errs() << X << "\n"

//===-- Statements dumps --===//

void CompoundStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, CompundStmt);
    DUMP_NEWLINE("");
    Indent.push_back(dump_indent);
    dump_indent++;
    for (auto &stmt : Stmts) {
        stmt->dump();
    }
    dump_indent--;
    Indent.pop_back();
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
    Indent.push_back(dump_indent);
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
    Indent.pop_back();
}

void ReturnStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, ReturnStmt);
    dump_indent++;
    DUMP_NEWLINE("");
    if (RetExpr != nullptr) {
        RetExpr->dump();
    }
    else {
        DUMP_WITH_IDENT(dump_indent, Void);
    }
    DUMP_NEWLINE("");
    dump_indent--;
}

void CallStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, CallStmt);
    DUMP_NEWLINE(Callee);
    dump_indent++;
    for (auto &args : Args) {
        args->dump();
    }
    dump_indent--;
}

void ArraySubscriptStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, ArraySubscript);
    dump_indent++;
    DUMP_NEWLINE("");
    Base->dump();
    for (const auto &idx : Idx)
        idx->dump();
    dump_indent--;
}

void DeclRefStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, DeclRef);
    DUMP_NEWLINE(Symbol);
}

void ::BinaryOperatorStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, Binary);
    const char *OpName;
    switch (Opcode) {
        case Add : OpName = "+"; break;
        case Sub : OpName = "-"; break;
        case Mul : OpName = "*"; break;
        case Div : OpName = "/"; break;
        case Assign : OpName = "="; break;
        case Greater : OpName = ">"; break;
        case Less : OpName = "<"; break;
        case Equal : OpName = "=="; break;
        case NotEqual : OpName = "!="; break;
        case GreaterEqual : OpName = ">="; break;
        case LessEqual : OpName = "<=";break;
        case And : OpName = "&&"; break;
        case Or : OpName = "||"; break;
    }
    DUMP_NEWLINE(OpName);
    Indent.push_back(dump_indent);
    dump_indent++;
    SubExprs[0]->dump();
    SubExprs[1]->dump();
    dump_indent--;
    Indent.pop_back();
}


void ::UnaryOperatorStmt::dump() {
    DUMP_WITH_IDENT(dump_indent, Binary);
    const char *OpName;
    switch (Opcode) {
        case Addr : OpName = "prefix '*'"; break;
    }
    DUMP_NEWLINE(OpName);
    Indent.push_back(dump_indent);
    dump_indent++;
    SubExpr->dump();
    dump_indent--;
    Indent.pop_back();
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
    Cond->dump();
    DUMP_WITH_IDENT(dump_indent, While);
    dump_indent++;
    DUMP_NEWLINE("");
    Cond->dump();
    Body->dump();
    dump_indent--;
}

void Cast::dump() { {
        DUMP_WITH_IDENT(dump_indent, Cast);
        DUMP_NEWLINE("");
}}


//===-- Declaration dumps --===//

void CompileUnitDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, CompileUnit);
    DUMP_NEWLINE(Name);
    Indent.push_back(dump_indent);
    dump_indent++;
    for (auto &decl : Decls) {
        decl->dump();
    }
    dump_indent--;
    Indent.pop_back();
}

void FunctionDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, Function);
    DUMP_NEWLINE(Name);
    Indent.push_back(dump_indent);
    dump_indent++;
    for (auto &param : Params) {
        param->dump();
    }
    if (hasBody()) {
        Indent.pop_back();
        Body->dump();
    }
    dump_indent--;
}

void VarDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, VarDecl);
    errs() << Name << "\n";
    if (getInit() != nullptr) {
        dump_indent++;
        DUMP_WITH_IDENT(dump_indent, Init);
        errs() << "\n";
        getInit()->dump();
        dump_indent--;
    }

}

void ParamDecl::dump() {
    DUMP_WITH_IDENT(dump_indent, ParamDecl);
    DUMP_NEWLINE(Name);
}

#endif