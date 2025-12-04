cmake_minimum_required(VERSION 4.0)
project(Maquina_virtual___Arquitectura_de_computadoras_2025 C)

set(CMAKE_C_STANDARD 11)

include_directories(mv/include)

add_executable(Maquina_virtual___Arquitectura_de_computadoras_2025
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
