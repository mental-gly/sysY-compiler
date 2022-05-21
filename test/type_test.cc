#include "logging.h"
#include "AST/TypeInfo.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>
using namespace llvm;

int main() {
    auto Context = std::make_unique<llvm::LLVMContext>();
    TypeContext::Init(Context.get());
    REGISTER_POINTER(int);
    REGISTER_POINTER(double);
    TypeInfo *int_pointer = TypeContext::find("int*");
    TypeInfo *double_pointer = TypeContext::find("double*");
    CHECK_NE(int_pointer, double_pointer);
    CHECK(!(TypeContext::checkEquivalence(int_pointer, double_pointer)));
}