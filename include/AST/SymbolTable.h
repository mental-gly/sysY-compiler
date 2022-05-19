#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

/// \file SymbolTable.h
/// \brief Symbol Table lookup Value by name
/// it maybe good to use llvm::StringRef and llvm::ilist for container,
/// with llvm::StringRef to pass symbols; but we want to lower the
/// testing troubles building with llvm libraries.

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringMap.h>
#include "logging.h"
#include <list>


/// \brief Handles symbol shadowing problem,
/// different from llvm::SymbolValueMap or llvm::SymbolTableList,
/// which offers `lookup` which use value default constructor if not found.
template <typename EntryType>
class SymbolTable {
    using ScopeType = llvm::StringMap<EntryType>;
    using ScopeListType = std::list< ScopeType* >;
public:
    SymbolTable() {
        auto InitGlobalScope = new ScopeType;
        ScopeValueMapList.push_front(InitGlobalScope);
    }
    ~SymbolTable() {
        ScopeValueMapList.erase(ScopeValueMapList.begin(), 
                                ScopeValueMapList.end());
    }
public:
    using iterator       = typename ScopeListType::iterator;
    using const_iterator = typename ScopeListType::const_iterator;
    iterator       begin()       { return ScopeValueMapList.begin(); }
    const_iterator begin() const { return ScopeValueMapList.begin(); }
    iterator       end()         { return ScopeValueMapList.end(); }
    const_iterator end()   const { return ScopeValueMapList.end(); }
public:
    /// \brief Enter a new scope; may have variable shadowing.
    void CreateScope() {
        auto new_scope = new ScopeType;
        ScopeValueMapList.push_front(new_scope);
    }
    /// \brief Leave current scope.
    void LeaveScope() {
        CHECK_GT(ScopeValueMapList.size() ,1UL) << "Cannot leave global scope!";
        ScopeValueMapList.erase(ScopeValueMapList.begin());      
    }

    bool isGlobalScope() const {
        return ScopeValueMapList.size() == 1;
    }

    void insertSymbol(llvm::StringRef symbol, const EntryType &val) {
        CHECK(ScopeValueMapList.front()->
            insert(std::make_pair(symbol, val)).second)
        << "Duplicated symbol " << symbol.str();
    }

    /// \brief lookup the symol table and return the entry;
    /// if not exist, throw error messages.
    EntryType lookup(llvm::StringRef symbol) {
        EntryType entry;
        for (auto& Scope : ScopeValueMapList) {
            if ((entry = Scope->lookup(symbol)) != EntryType()) 
                return entry;
        }
        LOG(FATAL) << "Undeclared symbol " << symbol.str();
        return EntryType();
    }

    EntryType &operator[](llvm::StringRef Key) {
        return (*begin())->operator[](Key);
    }
private:
    ScopeListType ScopeValueMapList;
};



#endif // COMPILER_SYMBOLTABLE_H