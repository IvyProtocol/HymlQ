cmake_minimum_required ( VERSION 4.2.1 )

set(CMAKE_COLOR_DIAGNOSTICS ON)

project( lib_ullix_
    VERSION 0.0.1
    LANGUAGES CXX
    DESCRIPTION ""
)

add_executable ( a.out )

target_compile_options( a.out PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wfloat-equal
  -Wshadow
  -Wold-style-cast
  -Wnon-virtual-dtor
  -Wunused
  -Woverloaded-virtual
  -Werror
  -Weverything
  -Wno-c++98-compat
  -Wno-switch-default
  -Wno-c++98-compat-pedantic
  -Wno-covered-switch-default
  -ferror-limit=999999
  -fdiagnostics-color
  -std=c++2c
)

target_sources( a.out PUBLIC
    src/Main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/FileHandler.cpp
    src/Transpiler.cpp
    # src/MetaProgram.cpp
)
target_include_directories(a.out PRIVATE
    include/
)
target_compile_features (a.out PRIVATE
    cxx_std_26
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-O3 -flto -march=native)
    add_link_options(-flto)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

target_link_options(a.out PRIVATE -lstdc++exp)
target_link_libraries(a.out PRIVATE -lstdc++exp)
