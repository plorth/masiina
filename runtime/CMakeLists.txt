CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(
  MasiinaRuntime
  VERSION 1.0.0
  DESCRIPTION "Runtime for Masiina virtual machine."
  LANGUAGES CXX C
)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILE(sysexits.h HAVE_SYSEXITS_H)
CHECK_FUNCTION_EXISTS(fork HAVE_FORK)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/masiina/runtime/config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/masiina/runtime/config.hpp
)

ADD_EXECUTABLE(
  masiina-runtime
  src/environment.cpp
  src/io.cpp
  src/main.cpp
  src/module.cpp
  src/parser.cpp
  src/routine.cpp
)

TARGET_COMPILE_OPTIONS(
  masiina-runtime
  PRIVATE
    -Wall -Werror
)

TARGET_COMPILE_FEATURES(
  masiina-runtime
  PRIVATE
    cxx_std_17
)

TARGET_INCLUDE_DIRECTORIES(
  masiina-runtime
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cget/include>
)

TARGET_LINK_DIRECTORIES(
  masiina-runtime
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cget/lib
)

TARGET_LINK_LIBRARIES(
  masiina-runtime
  plorth
)

SET_TARGET_PROPERTIES(
  masiina-runtime
  PROPERTIES
    OUTPUT_NAME masiina
)

INSTALL(
  TARGETS
    masiina-runtime
  RUNTIME DESTINATION
    bin
)

