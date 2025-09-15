#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char memoria[16384];
    int registros[32];
    int tablaSegmentos[8][2];
} maquinaVirtual;

void leerMV( maquinaVirtual *mv, FILE* arch);

void ejecutarMV(maquinaVirtual *mv);

void disassembler( maquinaVirtual mv );
