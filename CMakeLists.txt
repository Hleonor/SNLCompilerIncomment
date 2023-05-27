cmake_minimum_required(VERSION 3.24)
project(SNLCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(SNLCompiler main.cpp DesProcess.cpp LL1Process.cpp SemanticAnalysis.cpp wordAn.cpp)
