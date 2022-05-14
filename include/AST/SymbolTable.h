#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

/// \file SymbolTable.h
/// \brief Symbol Table lookup Value by name
/// it maybe good to use llvm::StringRef and llvm::ilist for container,
/// with llvm::StringRef to pass symbols; but we want to lower the
/// testing troubles building with llvm libraries.

#include <map>
#include <list>
#include "logging.h"

/// \brief Hash table map symbol name to some attribute
/// based on llvm::StringMap, 
template <typename T>
class SymbolTableEntry {
    using SymbolMapType = std::map<std::string, T>;
public:
    using iterator       = typename SymbolMapType::iterator;
    using const_iterator = typename SymbolMapType::const_iterator;

    iterator        begin()       { return vmap.begin(); }
    const_iterator  begin() const { return vmap.begin(); } 
    iterator        end()         { return vmap.end(); }
    const_iterator  end()   const { return vmap.end(); }

public:
    iterator find(const std::string &symbol) {
        return vmap.find(symbol);
    }
    const_iterator find(const std::string &symbol) const {
        return vmap.find(symbol);
    }
    std::pair<iterator, bool> insert(const std::string &key, const T &val) {
        return vmap.insert(std::make_pair(key, val));
    }
private:
    SymbolMapType vmap;
};


/// \brief Handles symbol shadowing problem,
/// different from llvm::SymbolValueMap or llvm::SymbolTableList,
/// which offers `lookup` which use value default constructor if not found.
template <typename EntryType>
class SymbolTable {
    using ScopeType = SymbolTableEntry<EntryType> *;
    using ScopeListType = std::list< ScopeType >;
public:
    SymbolTable() {
        auto InitGlobalScope = new SymbolTableEntry<EntryType>;
        ScopeValueMapList.push_front(InitGlobalScope);
    }
    ~SymbolTable() {
        ScopeValueMapList.clear();
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
        auto NewScope = new SymbolTableEntry<EntryType>;
        ScopeValueMapList.push_front(NewScope);
    }
    /// \brief Leave current scope.
    void LeaveScope() {
        CHECK_GE(ScopeValueMapList.size() ,1) << "Cannot leave global scope!";
        ScopeType CurScope = ScopeValueMapList.front();
        ScopeValueMapList.pop_front();      
        delete CurScope;  
    }

    bool isGlobalScope() const {
        return ScopeValueMapList.size() == 1;
    }

    void insertSymbol(const std::string &symbol, const EntryType &val) {
        CHECK(ScopeValueMapList.front()->insert(symbol, val).second)
        << "Duplicated symbol " << symbol;
    }

    /// \brief lookup the symol table and return the entry;
    /// if not exist, throw error messages.
    EntryType lookup(const std::string &symbol) {
        using Entry = typename SymbolTableEntry<EntryType>::iterator; 
        Entry entry;
        for (const auto& Scope : ScopeValueMapList) {
            if ((entry = Scope->find(symbol)) != Scope->end()) 
                return entry->second;
        }
        LOG(FATAL) << "Undeclared symbol " << symbol;
        return EntryType();
    }
private:
    ScopeListType ScopeValueMapList;
};



#endif // SYMBOLTABLE_H