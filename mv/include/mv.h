#ifndef MV_H
#define MV_H

#define MEM_SIZE 16384
#define NUM_REG 32
#define NUM_SEG 8

#include "operando.h"
#include "funciones.h"
#include "operando.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define LAR 0
#define MAR 1
#define MBR 2
#define IP  3
#define OPC 4
#define OP1 5
#define OP2 6

#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15
#define AC  16
#define CC  17
#define CS  26
#define DS  27

typedef struct {
    uint16_t base;
    uint16_t size;
} descriptor;

typedef struct {
    char memoria[MEM_SIZE];
    int registros[NUM_REG];
    descriptor  tablaSegmentos[NUM_SEG];
} maquinaVirtual;

int puntero(int posFisica);
int corrigeSize(int size);
void muestraCS(maquinaVirtual mv);

void setReg(maquinaVirtual *mv, int reg, int valor);
int getReg(maquinaVirtual *mv, int reg);

//lectura de memoria a la maquina virtual
void leerMV( maquinaVirtual *mv, FILE* arch); 

//ejecucion de la maquina virtual
void ejecutarMV(maquinaVirtual *mv);

//muestra del codigo ensamblador en pantalla
void disassembler( maquinaVirtual mv );

#endif // MV_H
