#ifndef OPERANDO_H
#define OPERANDO_H

typedef struct maquinaVirtual maquinaVirtual;
extern const char* mnemonicos[32];

typedef struct operando {
    char tipo;
    char registro;
    int desplazamiento; //o valor inmediato
} operando;

int getReg(maquinaVirtual *mv, int op);
int getInm(maquinaVirtual *mv, int op) ;
int getOPReg(maquinaVirtual *mv, operando op);
int getOPMem(maquinaVirtual *mv, operando op);
int getOp(maquinaVirtual *mv, int op);
void setOp(maquinaVirtual *mv, int op, int num);
void recuperaOperandos(maquinaVirtual *mv, operando *operandos,int ip);
int getMem(maquinaVirtual *mv, int op);
void imprimeOperando(operando op);
int decodificaOperando(maquinaVirtual *mv, int pos, int tipo, operando *op);



#endif // OPERANDO_H
