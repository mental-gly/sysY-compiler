#include "AST/TypeInfo.h"
#include <sstream>
std::vector<TypeInfo> TypeContext::type_table;


void TypeContext::Init(){
    // special type
    TypeContext::type_table.emplace_back(
        std::hash<std::string>()("void"), 0, TypeInfo::kVoid);
    // register primitive numeric types
    REGISTER_NUMERIC(float);   
    REGISTER_NUMERIC(double);
    REGISTER_NUMERIC(char);
    REGISTER_NUMERIC(unsigned char);
    REGISTER_NUMERIC(short);
    REGISTER_NUMERIC(unsigned short);
    REGISTER_NUMERIC(int);
    REGISTER_NUMERIC(unsigned int);
    REGISTER_NUMERIC(long long);
    REGISTER_NUMERIC(unsigned long long);
}


bool TypeContext::checkEquivalance(TypeInfo *Src, TypeInfo *Tgt) {
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
                            sizeof(void*), TypeInfo::kPointer);
    auto new_type = &type_table.back();
    new_type->Use[0] = base_type;
    return new_type;
}

TypeInfo *TypeContext::createArrayType(const std::string &name_key, size_t Length) {
    // construct array type name
    std::ostringstream ArrayOs;
    ArrayOs << name_key << "[" << Length << "]";
    auto type = find(ArrayOs.str());
    if (type != nullptr) return type;
    auto base_type = find(name_key);
    CHECK(base_type) << "Unknown type " << name_key;
    type_table.emplace_back(std::hash<std::string>()(ArrayOs.str()),
                            sizeof(base_type->ByteSize * Length), TypeInfo::kArrays);
    auto new_type = &type_table.back();
    new_type->Use[0] = base_type;
    return new_type;
}