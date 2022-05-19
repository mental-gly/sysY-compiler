llvm_map_components_to_libnames(LLVM_LIBS
    support core 
    asmprinter selectiondag codegen target mc)

set(AST_SOURCES
        TypeInfo.cc
        Stmt.cc
        Decl.cc
)

add_library(AST STATIC ${AST_SOURCES})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)