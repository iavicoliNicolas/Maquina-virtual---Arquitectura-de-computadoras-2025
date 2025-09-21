#ifndef OPERANDO_H
#define OPERANDO_H

typedef struct maquinaVirtual maquinaVirtual;

typedef struct operando {
    char tipo;
    char registro;
    char segmentoReg;
    int desplazamiento;
} operando;

extern const char* mnemonicos[32];

int getRegOp(maquinaVirtual *mv, operando op);
int getMem(maquinaVirtual *mv, operando op);
int getOp(maquinaVirtual *mv, operando op);   
void setOp(maquinaVirtual *mv, operando op, int valor);
void recuperaOperandos(maquinaVirtual *mv,operando *o,int ip);
void imprimeOperando(operando op);
int decodificaOperando(maquinaVirtual *mv, int pos, int tipo, operando *op);



#endif // OPERANDO_H
