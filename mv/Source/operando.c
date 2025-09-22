#include <stdio.h>
#include "mv.h"
#include "operando.h"
#include "disassembler.h" // para poder escribir los nombres de los reg



void imprimeOperando(operando op) {
    switch (op.tipo) {
        case 0: // ninguno
            break;

        case 1: // registro
            if (op.registro >= 0 && op.registro < 32 && nombres_registros[op.registro])
                printf("%s", nombres_registros[op.registro]);
            else
                printf("reg%d", op.registro); // por si es reservado
            break;

        case 2: // inmediato (valor)
            printf("%d", op.desplazamiento);
            break;

        case 3: // memoria 
            printf("[");
            if (op.registro != 0 && op.registro < 32 && nombres_registros[op.registro]) {
                printf("%s", nombres_registros[op.registro]);
                if (op.desplazamiento) {
                    printf(" + %d", op.desplazamiento);
                }
            } else {
                // acceso directo a memoria (sin registro base)
                printf("%d", op.desplazamiento);
            }
            printf("]");
            break;
    }
}




int getRegOp(maquinaVirtual *mv, operando op) { return 0; }
int getMem(maquinaVirtual *mv, operando op) { return 0; }
int getOp(maquinaVirtual *mv, operando op) { return 0; }
void setOp(maquinaVirtual *mv, operando op, int val) {}
void recuperaOperandos(maquinaVirtual *mv, operando *o, int ip) {}