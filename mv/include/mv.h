#ifndef MV_H
#define MV_H

#include "operando.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char memoria[16384];
    int registros[32];
    int tablaSegmentos[8][2];
} maquinaVirtual;

typedef char CodOperacion[5];
void iniciaNombresOperaciones( CodOperacion nombresOperaciones[32] );

typedef char CodRegistro[5];
void iniciaRegNombres( CodOperacion nombresRegistros[32][4] );



void muestInme(char muest[]);
void muestRegi(char muest[], char *NombreRegistro[][4]);
void muestMem(char muest[], char *NomReg[][4]);
int corrigeSize(int size);
void leerMV( maquinaVirtual *mv, FILE* arch);
void muestraCS(maquinaVirtual mv);


void ejecutarMV(maquinaVirtual *mv);

void disassembler( maquinaVirtual mv );

#endif // MV_H
