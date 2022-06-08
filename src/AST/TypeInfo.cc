#include "AST/TypeInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include <sstream>
using namespace llvm;
std::vector<TypeInfo> TypeContext::type_table;
LLVMContext *TypeContext::context = nullptr;

void TypeContext::Init(LLVMContext *Context){
    // Set LLVM Context.
    context = Context;
    // special type
    TypeContext::type_table.emplace_back(
        std::hash<std::string>()("void"), 0, TypeInfo::kVoid);
    SetLLVMType(&type_table.back(), Type::getVoidTy(*context));
    // register primitive numeric types
    SetLLVMType(REGISTER_NUMERIC(float),             Type::getFloatTy(*context));
    SetLLVMType(REGISTER_NUMERIC(double),            Type::getDoubleTy(*context));
    SetLLVMType(REGISTER_NUMERIC(char),              Type::getInt8Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(unsigned char),     Type::getInt8Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(short),             Type::getInt16Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(unsigned short),    Type::getInt16Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(int),               Type::getInt32Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(unsigned int),      Type::getInt32Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(long long),         Type::getInt64Ty(*context));
    SetLLVMType(REGISTER_NUMERIC(unsigned long long),Type::getInt64Ty(*context));
    // String Literal char *
    SetLLVMType(REGISTER_POINTER("char"),              Type::getInt8PtrTy(*context));
}


bool TypeContext::checkEquivalence(TypeInfo *Src, TypeInfo *Tgt) {
    switch (Src->Kind) {
        // primitive type 
        case TypeInfo::kNumeric:
            return (Src == Tgt); 
        // pointer type, check dereferenced type 
        case TypeInfo::kPointer:
            return (Tgt->Kind == TypeInfo::kPointer &&  
                    Src->Use[0] == Tgt->Use[0]);
        case TypeInfo::kAlias:
            return (Src->Use[0] == Tgt ||
                    Src->Use[0] == Tgt->Use[0]);
        default:
            return false;
    }
}


TypeInfo *TypeContext::find(const std::string &name_key) {
    size_t hashed_name = std::hash<std::string>()(name_key);
    auto type = std::find_if(type_table.cbegin(), type_table.cend(),
                                    [&](const TypeInfo &Info){ return Info.NameHash == hashed_name; });
    if (type != type_table.end()) 
        return const_cast<TypeInfo*>(&(*type));
    else return nullptr;
}

TypeInfo *TypeContext::createNumericType(const std::string &name_key, size_t size) {
    auto type = find(name_key);
    if (type != nullptr) return type;
    type_table.emplace_back(std::hash<std::string>()(name_key), 
                            size, TypeInfo::kNumeric);
    return &type_table.back();
}

TypeInfo *TypeContext::createPointerType(const std::string &name_key) {
    std::ostringstream PointerOs;
    PointerOs << name_key << "*";
    auto type = find(PointerOs.str());
    if (type != nullptr) return type;
    auto base_type = find(name_key);
    CHECK(base_type) << "Unknown type " << name_key;
    type_table.emplace_back(std::hash<std::string>()(PointerOs.str()),
                            sizeof(void*), TypeInfo::kPointer, 1);
    auto new_type = &type_table.back();
    new_type->Element = base_type;
    SetLLVMType(new_type, PointerType::get(base_type->Type, 0));
    return new_type;
}

TypeInfo *TypeContext::createArrayType(const std::string &name_key, size_t Length) {
    // construct array type name
    std::ostringstream ArrayOs;
    ArrayOs << name_key << "[" << Length << "]";
    auto type = find(ArrayOs.str());
    if (type != nullptr) return type;
    // register new type.
    auto base_type = find(name_key);
    CHECK(base_type) << "Unknown type " << name_key;
    type_table.emplace_back(std::hash<std::string>()(ArrayOs.str()),
                            sizeof(base_type->ByteSize * Length), TypeInfo::kArrays);
    auto new_type = &type_table.back();
    new_type->Element = base_type;

    if (Length > 0)
        // A true array type.
        SetLLVMType(new_type, ArrayType::get(base_type->Type, Length));
    else
        // A function parameter type, treat as pointer.
        SetLLVMType(new_type, PointerType::get(base_type->Type, 0));
    return new_type;
}