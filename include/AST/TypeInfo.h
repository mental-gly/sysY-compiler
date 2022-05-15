#ifndef TYPEINFO_H
#define TYPEINFO_H

/// \file TypeInfo.h
/// \brief TypeInfo class describe type equivalence
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "logging.h"

struct TypeInfo {
    TypeInfo() = delete;
    TypeInfo(size_t NameHash_, size_t ByteSize_, uint32_t Kind_, size_t Uses = 0) 
        : NameHash(NameHash_), ByteSize(ByteSize_), Kind(Kind_)
    {  
        if (Uses != 0) 
            Use = new TypeInfo*[Uses];
    }
    ~TypeInfo() { delete Use; }

    /// \todo set type use
    enum : uint32_t {
        kUnkown,
        kVoid,
        kNumeric,
        kPointer,
        kArrays,
        kStruct, // Use std::aligned_union_t
        kAlias,
        kFunction, // Function type
        NUM_TYPE
    };
    // For search the type
    size_t NameHash; 
    // alloca space info
    size_t ByteSize;
    uint32_t Kind;
    // struct type member variable types
    // If this type is defined with typedef (kAlias), parent is the original type.
    // IF this type is a pointer (kPointer), parent is the dereferenced class. 
    TypeInfo **Use {nullptr}; 
};



class TypeContext {
public:
    /// \brief register primitive types.
    static void Init();
    static bool checkEquivalance(TypeInfo *Src, TypeInfo *Tgt); 

    static TypeInfo *find(const std::string &name_key);

    /// \brief register numeric type with \p name and \p size
    static TypeInfo &createNumericType(const std::string &name_key, size_t size);

    /// \brief register pointer type \p T* given type name \p T
    static TypeInfo &createPointerType(const std::string &name_key);

    [[maybe_unused]] static TypeInfo &createStructType(const std::string &name_key, const std::vector<TypeInfo *> members) {
        LOG(FATAL) << "Struct is not supported!";
    }
private:
    // Type table only operate at one end and
    // requires fast append and remove operation.
    static std::vector<TypeInfo> type_table;
};


#define REGISTER_NUMERIC(X) TypeContext::createNumericType(#X, sizeof(X))
#define REGISTER_POINTER(X) TypeContext::createPointerType(#X)




#endif // TYPEINFO_H