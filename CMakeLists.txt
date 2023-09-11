cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER_VERSION "10.1.0")

include(FetchContent)
FetchContent_Declare(
  googletest
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(
  AndOrDagTest
  AndOrDagTest.cpp AndOrDag.cpp Util.cpp CSR.cpp
  parser/rpqBaseListener.cpp parser/rpqBaseVisitor.cpp parser/rpqLexer.cpp parser/rpqListener.cpp parser/rpqParser.cpp parser/rpqVisitor.cpp
)
add_executable(
  chooseMatViewsTheoCompare
  chooseMatViewsTheoCompare.cpp AndOrDag.cpp Util.cpp CSR.cpp
  parser/rpqBaseListener.cpp parser/rpqBaseVisitor.cpp parser/rpqLexer.cpp parser/rpqListener.cpp parser/rpqParser.cpp parser/rpqVisitor.cpp
)
target_include_directories(AndOrDagTest PRIVATE /home/pangyue/gstore/tools/antlr4-cpp-runtime-4/runtime/src/)
target_include_directories(chooseMatViewsTheoCompare PRIVATE /home/pangyue/gstore/tools/antlr4-cpp-runtime-4/runtime/src/)
target_link_libraries(
  AndOrDagTest
  PRIVATE /home/pangyue/gstore/lib/libantlr4-runtime.a GTest::gtest_main
)
target_link_libraries(
  chooseMatViewsTheoCompare
  PRIVATE /home/pangyue/gstore/lib/libantlr4-runtime.a
)
target_compile_options(AndOrDagTest PRIVATE -g -Wall -DTEST)
target_compile_options(chooseMatViewsTheoCompare PRIVATE -g -Wall -O2)

include(GoogleTest)
gtest_discover_tests(AndOrDagTest)