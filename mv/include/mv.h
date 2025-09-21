#ifndef MV_H
#define MV_H

#include <stdio.h>
#include <stdlib.h>
#include "operando.h"

typedef struct maquinaVirtual {
    char memoria[16384];
    int registros[32];
    int tablaSegmentos[8][2];
} maquinaVirtual;

#define LAR 0
#define MAR 1
#define MBR 2
#define IP  3
#define OPC 4
#define OP1 5
#define OP2 6

#define AC 16
#define CC 17

#define CS 26
#define DS 27



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
void disassembler( maquinaVirtual *mv );

void leerInstruccion(maquinaVirtual *mv, operando *op);

#endif // MV_H