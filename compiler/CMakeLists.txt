CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(
  MasiinaCompiler
  VERSION 1.0.0
  DESCRIPTION "Compiler for Masiina virtual machine."
  LANGUAGES CXX C
)

INCLUDE(CheckIncludeFile)

CHECK_INCLUDE_FILE(sysexits.h HAVE_SYSEXITS_H)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/masiina/compiler/config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/masiina/compiler/config.hpp
)

ADD_EXECUTABLE(
  masiina-compiler
  src/compiler.cpp
  src/instruction.cpp
  src/io.cpp
  src/main.cpp
  src/module.cpp
  src/unit.cpp
)

TARGET_COMPILE_OPTIONS(
  masiina-compiler
  PRIVATE
    -Wall -Werror
)

TARGET_COMPILE_FEATURES(
  masiina-compiler
  PRIVATE
    cxx_std_17
)

TARGET_INCLUDE_DIRECTORIES(
  masiina-compiler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cget/include>
)

SET_TARGET_PROPERTIES(
  masiina-compiler
  PROPERTIES
    OUTPUT_NAME masiinac
)

INSTALL(
  TARGETS
    masiina-compiler
  RUNTIME DESTINATION
    bin
)
