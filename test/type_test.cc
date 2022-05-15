#include "logging.h"
#include "AST/TypeInfo.h"

int main() {
    TypeContext::Init();
    REGISTER_POINTER(int);
    REGISTER_POINTER(double);
    TypeInfo *int_pointer = TypeContext::find("int*");
    TypeInfo *double_pointer = TypeContext::find("double*");
    CHECK_NE(int_pointer, double_pointer);
    CHECK(!(TypeContext::checkEquivalance(int_pointer, double_pointer))); 
}