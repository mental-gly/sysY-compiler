cmake_minimum_required(VERSION 3.14)
project(compiler)
# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

# compile platform info
message(STATUS "CMake_HOST_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "CMAKE_HOST_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION}")
# backtrace need Linux syscall
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_compile_definitions(NEED_LOG_STACK_TRACE)
endif()

# compiler info
message(STATUS "Found C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Found CXX Compiler: ${CMAKE_CXX_COMPILER}")

if (BUILD_WITH_LLVM)
    # build with llvm
    find_package(LLVM CONFIG REQUIRED)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    message(STATUS "Using LLVM include directory: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "Using LLVM link directory: ${LLVM_LIBRARY_DIRS}")
    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LINK_DIRECTORIES})
    add_definitions(${LLVM_DEFINITIONS})

    # llvm libraries use built-in rtti
    add_compile_options(-fno-exceptions -fno-rtti)
    add_compile_options(-ltinfo -lpthread)
endif()

# set default cmake build type as Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(include)
add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    enable_testing()
    add_compile_options(-Wall -Wextra -g)
    add_subdirectory(test)
endif()