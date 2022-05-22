set(PARSER_SOURCE
    ${PROJECT_SOURCE_DIR}/src/Lexer-Parser/lex.yy.cc
    ${PROJECT_SOURCE_DIR}/src/Lexer-Parser/parser.tab.cc
)
set_source_files_properties(${PARSER_SOURCE} GENERATED)

add_custom_command(
        OUTPUT parser.tab.cc parser.tab.hh
        DEPENDS parser.y
        COMMAND bison -d parser.y -L C++
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/Lexer-Parser
)

add_custom_command(
        OUTPUT lex.yy.cc
        DEPENDS parser.tab.hh sysy.l
        COMMAND flex -+ sysy.l
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/Lexer-Parser
)

add_library(Parser STATIC "")
target_sources(Parser PRIVATE ${PARSER_SOURCE})
