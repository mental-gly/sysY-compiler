FILE(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*test.cc)

llvm_map_components_to_libnames(LLVM_ANALYSIS analysis)

foreach (test_source ${TEST_SOURCES})
    # Create test suit
    get_filename_component(test_filename ${test_source} NAME)
    string(REPLACE ".cc" "" test_name ${test_filename})
    MESSAGE(STATUS "Create test suit: ${test_name}")

    # Add the test target separately and as part of "make check-tests".
    add_executable(${test_name} EXCLUDE_FROM_ALL ${test_source})

    # Set test target properties and dependencies.
    set_target_properties(${test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${test_name}
            )
    target_link_libraries(${test_name} AST)
    # Add the test under CTest.
    add_test(${test_name} ${CMAKE_BINARY_DIR}/test/${test_name})
endforeach (test_source ${TEST_SOURCES})

target_link_libraries(StringLiteral_test ${LLVM_ANALYSIS})
target_link_libraries(parser_test Parser)