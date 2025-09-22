#ifndef OPERANDO_H
#define OPERANDO_H

typedef struct maquinaVirtual maquinaVirtual;
extern const char* mnemonicos[32];

typedef struct operando {
    char tipo;
    char registro;
    int desplazamiento; //o valor inmediato
} operando;


int getOPReg(maquinaVirtual *mv, operando op);
int getOPMem(maquinaVirtual *mv, operando op);
int getOp(maquinaVirtual *mv, operando op);
void setOp(maquinaVirtual *mv, operando op, int num);
void recuperaOperandos(maquinaVirtual *mv, int *operandos,int ip);


void imprimeOperando(operando op);
int decodificaOperando(maquinaVirtual *mv, int pos, int tipo, operando *op);



#endif // OPERANDO_H
