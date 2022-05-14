if (BUILD_WITH_LLVM)
llvm_map_componenets_to_libnames(LLVM_LIBS 
    support core 
    asmprinter selectiondag codegen target mc)
endif()

set(AST_SOURCES
    TypeInfo.cc
)

add_library(AST STATIC ${AST_SOURCES})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)