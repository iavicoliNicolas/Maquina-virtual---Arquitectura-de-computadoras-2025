#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char memoria[16384];
    int registros[32];
    int tablaSegmentos[8][2];
} maquinaVirtual;

int corrigeSize(int size);
void leerMV( maquinaVirtual *mv, FILE* arch);
void muestraCS(maquinaVirtual mv);


void ejecutarMV(maquinaVirtual *mv);

void disassembler( maquinaVirtual mv );
