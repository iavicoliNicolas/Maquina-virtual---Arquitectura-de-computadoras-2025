#ifndef FUNCIOONES_H
#define FUNCIOONES_H
#include "mv.h"
#include "operando.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//funciones de la maquina virtual
//tipo de funcion para operaciones de la maquina virtual
typedef void (*Toperaciones)(maquinaVirtual *, int*);
//typedef void (*funcionSys)(maquinaVirtual *, int);
typedef void (*funcionSys)(maquinaVirtual *);

void cargaVF(Toperaciones *);
void loadSYSOperationArray(funcionSys *vecLlamadas);

void setLAR(maquinaVirtual *mv, int dirL);
void setMAR(maquinaVirtual *mv, int nbytes, int dirFisica);
void setMBR(maquinaVirtual *mv, int valor);

void setRegOP(maquinaVirtual *mv, int reg, operando valor, int tipo);


void setCC(maquinaVirtual *mv, int resultado);
//funciones de assembler
void MOV(maquinaVirtual *mv, int *op);
void ADD(maquinaVirtual *mv, int *op);
void SUB(maquinaVirtual *mv, int *op);
void SWAP(maquinaVirtual *mv, int *op);;
void MUL(maquinaVirtual *mv, int *op);
void DIV(maquinaVirtual *mv, int *op);
void CMP(maquinaVirtual *mv, int *op);
void SHL(maquinaVirtual *mv, int *op);
void SAR(maquinaVirtual *mv, int *op);
void SHR(maquinaVirtual *mv, int *op);
void AND(maquinaVirtual *mv, int *op);
void OR(maquinaVirtual *mv, int *op);
void XOR(maquinaVirtual *mv, int *op);
void RND(maquinaVirtual *mv, int *op);
void SYS(maquinaVirtual *mv, int *op);
void JMP(maquinaVirtual *mv, int *op);
void JZ(maquinaVirtual *mv, int *op);
void JP(maquinaVirtual *mv, int *op);
void JN(maquinaVirtual *mv, int *op);
void JNZ(maquinaVirtual *mv, int *op);
void JNP(maquinaVirtual *mv, int *op);
void JNN(maquinaVirtual *mv, int *op);
void LDL(maquinaVirtual *mv, int *op);
void LDH(maquinaVirtual *mv, int *op);
void NOT(maquinaVirtual *mv, int *op);
void PUSH(maquinaVirtual *mv, int *op);
void POP(maquinaVirtual *mv, int *op);
void CALL(maquinaVirtual *mv, int *op);
void RET(maquinaVirtual *mv, int *op);
void STOP(maquinaVirtual *mv, int *op);

//funciones de la llamada sys
void readSys(maquinaVirtual *mv,int aux);
void writeSys(maquinaVirtual *mv,int aux);

void ejecutarOperacion(maquinaVirtual *mv, int codigo, operando *op);
#endif // FUNCIOONES_H
