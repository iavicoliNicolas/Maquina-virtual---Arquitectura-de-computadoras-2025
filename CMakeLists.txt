cmake_minimum_required(VERSION 3.16)
project(mv2025 C)    # ← nombre interno más corto

set(CMAKE_C_STANDARD 11)
include_directories(mv/include)

add_executable(mv2025
        mv/include/disassembler.h
        mv/include/funciones.h
        mv/include/mv.h
        mv/include/operando.h
        mv/include/paramsegment.h
        mv/Source/disassembler.c
        mv/Source/funciones.c
        mv/Source/main.c
        mv/Source/mv.c
        mv/Source/operando.c
        mv/Source/paramsegment.c
        mv/tests/test.c)

set_target_properties(mv2025 PROPERTIES OUTPUT_NAME "vmx")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})