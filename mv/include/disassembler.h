#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H
#include <stdio.h>
#include "mv.h"
#include "operando.h"

typedef struct{
    unsigned int ipInicio;
    unsigned int ipFinal;
    unsigned int codigoOperacion;
    operando operandos[2];
}InstruccionDisassembler;


extern const char* nombres_registros[32];
typedef void (*funcionDisassembler)(InstruccionDisassembler );

void cargaVectorDisassembler(funcionDisassembler *v);

void disassembler(maquinaVirtual *mv);
#endif
