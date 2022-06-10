
set(DRIVER_SOURCE
        Driver.cc
)

add_executable(sycc ${DRIVER_SOURCE})
set_target_properties(
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_link_libraries(sycc  AST Parser LLVM)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
