
add_executable(symbol_table_test symbol_table_test.cc)
# Set test target properties and dependencies.
set_target_properties(symbol_table_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
        COMMAND symbol_table_test
        )
add_test(symbol_table_test ${CMAKE_BINARY_DIR}/test/symbol_table_test)