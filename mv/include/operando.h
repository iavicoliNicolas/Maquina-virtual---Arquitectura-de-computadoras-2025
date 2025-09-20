#ifndef OPERANDO_H
#define OPERANDO_H

#include "mv.h"


typedef struct  {
    char tipo;
    char registro;
    char segmentoReg;
    int desplazamiento;
} operando;

int getReg(maquinaVirtual *mv, operando op);
int getMem(maquinaVirtual *mv, operando op);
int getOp(maquinaVirtual *mv, operando op);
void setOp(maquinaVirtual *mv, operando op, int);
void recuperaOperandos(maquinaVirtual *mv,operando *o,int ip);
void imprimeOperando(operando op);


#endif // OPERANDO_H
