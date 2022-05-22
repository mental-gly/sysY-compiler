#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "logging.h"
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include <memory>
using namespace llvm;


int main() {
    auto Unit = new CompileUnitDecl("If.c");
    TypeContext::Init(Unit->getContext());
    auto IntType = TypeContext::find("int");
    auto paraN = new ParamDecl(IntType, "num");
    // function : int if_test(int num)
    auto F = new FunctionDecl(IntType, "If_test", paraN);

    // if (num > 1)
    auto Cond = new BinaryOperatorStmt(BinaryOperatorStmt::Greater,
                                       new DeclRefStmt("num"),
                                       new IntegerLiteral(IntType, 1));
    // return 1
    auto Then = new ReturnStmt(new IntegerLiteral(IntType, -1, true));
    // num = num + 1
    auto Else = new BinaryOperatorStmt(BinaryOperatorStmt::Assign,
                                   new DeclRefStmt("num"),
                                   new BinaryOperatorStmt(BinaryOperatorStmt::Add,
                                                      new DeclRefStmt("num"),
                                                      new IntegerLiteral(IntType, 1)));

    auto If = new IfStmt(Cond, Then, Else);
    // return num;
    auto Ret = new ReturnStmt(new DeclRefStmt("num"));

    If->Next = Ret;
    auto Body = new CompoundStmt(If);
    F->setBody(Body);
    // empty F2, but same parameter name.
    auto F2 = new FunctionDecl(IntType, "If_test2", paraN);
    F2->setBody(new CompoundStmt);
    Unit->CreateSubDecls(F);
    Unit->CreateSubDecls(F2);
    Unit->dump();
    Unit->CodeGen();
    Unit->print();
}