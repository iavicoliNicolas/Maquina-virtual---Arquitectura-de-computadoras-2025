#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdio.h>
#include "mv.h"

typedef struct{
    unsigned int ipInicio;
    unsigned int ipFinal;
    unsigned int codigoOperacion;
    operando operandos[2];
}InstruccionDisassembler;

typedef void (*funcionDisassembler)(InstruccionDisassembler );

void cargaVectorDisassembler(funcionDisassembler *v);

void imprimeMOV(InstruccionDisassembler disInstruccion);
void imprimeADD(InstruccionDisassembler disInstruccion);
void imprimeSUB(InstruccionDisassembler disInstruccion);
void imprimeSWAP(InstruccionDisassembler disInstruccion);
void imprimeMUL(InstruccionDisassembler disInstruccion);
void imprimeDIV(InstruccionDisassembler disInstruccion);
void imprimeCMP(InstruccionDisassembler disInstruccion);
void imprimeSHL(InstruccionDisassembler disInstruccion);
void imprimeSAR(InstruccionDisassembler disInstruccion);
void imprimeSHR(InstruccionDisassembler disInstruccion);
void imprimeAND(InstruccionDisassembler disInstruccion);
void imprimeOR(InstruccionDisassembler disInstruccion);
void imprimeXOR(InstruccionDisassembler disInstruccion);
void imprimeSYS(InstruccionDisassembler disInstruccion);
void imprimeJMP(InstruccionDisassembler disInstruccion);
void imprimeJZ(InstruccionDisassembler disInstruccion);
void imprimeJP(InstruccionDisassembler disInstruccion);
void imprimeJN(InstruccionDisassembler disInstruccion);
void imprimeJNZ(InstruccionDisassembler disInstruccion);
void imprimeJNP(InstruccionDisassembler disInstruccion);
void imprimeJNN(InstruccionDisassembler disInstruccion);
void imprimeLDL(InstruccionDisassembler disInstruccion);
void imprimeLDH(InstruccionDisassembler disInstruccion);
void imprimeRND(InstruccionDisassembler disInstruccion);
void imprimeNOT(InstruccionDisassembler disInstruccion);
void imprimeSTOP(InstruccionDisassembler disInstruccion);



void obtieneTAG(char reg,char segmento,char nombre[]);

#endif
