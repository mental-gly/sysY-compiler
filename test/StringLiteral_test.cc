#include "AST/Stmt.h"
#include "AST/Decl.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


int main() {
    auto Unit = new CompileUnitDecl("literal.cc");
    auto Literal = new ::StringLiteral("Hello World");
    Literal->CodeGen(Unit);
    // The global string literal is a internal value.
    auto StringV = Unit->getModule()->getGlobalVariable(Literal->getVal(), true);
    StringV->getType()->print(errs(), true);
    errs() << "\n";
    StringV->getType()->getElementType()->print(errs(), true);
    errs() << "\n";
    auto builder = Unit->getBuilder();
    auto context = Unit->getContext();
    auto GEPInst = builder->CreateGEP(StringV, ConstantInt::get(Type::getInt64Ty(*context), 1));
    GEPInst->print(errs(), true);
    errs() << "\n";
    Unit->getModule()->print(errs(), nullptr);
}