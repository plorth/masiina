CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(
  Masiina
  VERSION 1.0.0
  DESCRIPTION "Virtual machine for Plorth programming language."
  LANGUAGES CXX
)

ADD_SUBDIRECTORY(compiler)
ADD_SUBDIRECTORY(runtime)
