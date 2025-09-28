#ifndef MV_H
#define MV_H

#include <stdio.h>
#include <stdlib.h>
#include "operando.h"
#include "funciones.h"

#define MAX_MEM 16384
#define MAX_REG 32
#define MAX_SEG 8

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

#define AC 16
#define CC 17

#define CS 26
#define DS 27

typedef struct maquinaVirtual {
    unsigned char memoria[MAX_MEM];
    int registros[MAX_REG];
    int tablaSegmentos[MAX_SEG][2];
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
int logicoAFisico(maquinaVirtual *mv, int direccionLogica);
#endif // MV_H

