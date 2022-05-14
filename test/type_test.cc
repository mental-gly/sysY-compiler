#include "logging.h"
#include "AST/TypeInfo.h"

int main() {
    TypeContext::Init();
    REGISTER_POINTER(int);
    REGISTER_POINTER(double);
    TypeInfo *int_pointer = TypeContext::findTypeByName("int*");
    TypeInfo *double_pointer = TypeContext::findTypeByName("double*");
    CHECK_NE(int_pointer, double_pointer);
    CHECK(!(TypeContext::checkTypeEquivalance(int_pointer, double_pointer))); 
}