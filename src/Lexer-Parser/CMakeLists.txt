

find_package(BISON)
find_package(FLEX)
BISON_TARGET(CParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc
        COMPILE_FLAGS
        DEFINES_FILE  ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.hh
)
FLEX_TARGET(CLex sysy.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc
        COMPILE_FLAGS
)
ADD_FLEX_BISON_DEPENDENCY(CLex CParser)

add_library(Parser STATIC ${BISON_CParser_OUTPUTS} ${FLEX_CLex_OUTPUTS})
target_link_libraries(Parser INTERFACE AST)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
