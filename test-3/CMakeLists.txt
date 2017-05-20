cmake_minimum_required(VERSION 3.4.3)
project(tool)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility-inlines-hidden -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-maybe-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -Werror=date-time -std=c++11   -fno-exceptions -fno-rtti")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${Clang_DEFINITIONS})

# Now build our tools
add_executable(${PROJECT_NAME} tool.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader all)

set(CLANG_LIBS
clangAnalysis
clangARCMigrate
clangAST
clangASTMatchers
clangBasic
clangCodeGen
clangDriver
clangDynamicASTMatchers
clangEdit
clangFormat
clangFrontend
clangFrontendTool
clangIndex
clangLex
clangParse
clangRewrite
clangRewriteFrontend
clangSema
clangSerialization
clang
clangStaticAnalyzerCheckers
clangStaticAnalyzerCore
clangStaticAnalyzerFrontend
clangTooling
clangToolingCore
)

set(CLANG_LIBS 
clangTooling 
clangFrontendTool 
clangFrontend 
clangDriver 
clangSerialization 
clangCodeGen 
clangParse 
clangSema 
clangStaticAnalyzerFrontend 
clangStaticAnalyzerCheckers 
clangStaticAnalyzerCore 
clangAnalysis 
clangARCMigrate 
clangRewrite 
clangRewriteFrontend 
clangEdit 
clangAST 
clangLex 
clangBasic 
clang
)


# Link against LLVM libraries
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

target_link_libraries(${PROJECT_NAME} ${CLANG_LIBS})
