#ifndef FUNCIOONES_H
#define FUNCIOONES_H

#include "mv.h"
#include "operando.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int Sistema; //auxiliar 


//funciones de la maquina virtual
//tipo de funcion para operaciones de la maquina virtual
typedef void (*Toperaciones)(maquinaVirtual *, operando *);
typedef void (*funcionSys)(maquinaVirtual *, Sistema);

void cargaVF(Toperaciones *);
void loadSYSOperationArray(funcionSys *vecLlamadas);

void setLAR(maquinaVirtual *mv, int valor);
void setMAR(maquinaVirtual *mv, int valor);
void setMBR(maquinaVirtual *mv, int valor);

void setOP1(maquinaVirtual *mv, operando valor, int tipo);
void setOP2(maquinaVirtual *mv, operando valor, int tipo);

void setCC(maquinaVirtual *mv, int resultado);
//funciones de assembler
void MOV(maquinaVirtual *mv, operando *op);
void ADD(maquinaVirtual *mv, operando *op);
void SUB(maquinaVirtual *mv, operando *op);
void SWAP(maquinaVirtual *mv, operando *op);
void MUL(maquinaVirtual *mv, operando *op);
void DIV(maquinaVirtual *mv, operando *op);
void CMP(maquinaVirtual *mv, operando *op);
void SHL(maquinaVirtual *mv, operando *op);
void SAR(maquinaVirtual *mv, operando *op);
void SHR(maquinaVirtual *mv, operando *op);
void AND(maquinaVirtual *mv, operando *op);
void OR(maquinaVirtual *mv, operando *op);
void XOR(maquinaVirtual *mv, operando *op);
void RND(maquinaVirtual *mv, operando *op);
void SYS(maquinaVirtual *mv, operando *op);
void JMP(maquinaVirtual *mv, operando *op);
void JZ(maquinaVirtual *mv, operando *op);
void JP(maquinaVirtual *mv, operando *op);
void JN(maquinaVirtual *mv, operando *op);
void JNZ(maquinaVirtual *mv, operando *op);
void JNP(maquinaVirtual *mv, operando *op);
void JNN(maquinaVirtual *mv, operando *op);
void LDL(maquinaVirtual *mv, operando *op);
void LDH(maquinaVirtual *mv, operando *op);
void NOT(maquinaVirtual *mv, operando *op);
void STOP(maquinaVirtual *mv, operando *op);

//funciones de la llamada sys
void readSys(maquinaVirtual *mv,Sistema aux);
void writeSys(maquinaVirtual *mv,Sistema aux);

#endif // FUNCIOONES_H