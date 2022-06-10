llvm_map_components_to_libnames(LLVM_LIBS
    support core analysis
)

set(AST_SOURCES
        TypeInfo.cc
        Stmt.cc
        Decl.cc
        dump.cc
        Analysis.cc
)

add_library(AST STATIC ${AST_SOURCES})
target_link_libraries(AST INTERFACE ${LLVM_LIBS}
        )
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
