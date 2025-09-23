#ifndef OPERANDO_H
#define OPERANDO_H

#include "mv.h"


typedef struct  {
    char tipo;
    char registro;
    int desplazamiento; //o valor inmediato
} operando;

int getOPReg(maquinaVirtual *mv, operando op);
int getOPMem(maquinaVirtual *mv, operando op);
int getOp(maquinaVirtual *mv, int op);
void setOp(maquinaVirtual *mv, int op, int num);
void recuperaOperandos(maquinaVirtual *mv, operando *operandos,int ip);
void imprimeOperando(operando op);


#endif // OPERANDO_H
