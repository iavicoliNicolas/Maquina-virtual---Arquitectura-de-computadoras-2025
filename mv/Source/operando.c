#include "operando.h"

//obtiene el valor del operando de registro
int getOpReg(maquinaVirtual *mv, operando op) {

}

//obtiene el valor del operando de memoria
int getOpMem(maquinaVirtual *mv, operando op) {

}

//obtiene el valor del operando segun su tipo
int getOp(maquinaVirtual *mv, operando op) {

}

void setOp(maquinaVirtual *mv, operando op, int num) {

}
void recuperaOperandos(maquinaVirtual *mv, operando *operandos, int ip) {
    
    char aux;
    int auxInt;

    
    if (operandos[0].tipo != 0) { //si el operando
        ip++;
        if (operandos[0].tipo == 1) { //si es de registro

            aux = mv->memoria[ip];
            aux = aux & 0x0F;
            operandos[0].registro = aux;

        } else if (operandos[0].tipo == 2) { //si es inmediato

            auxInt |= mv->memoria[ip];
            auxInt |= mv->memoria[ip + 1] & 0x00FF;
            operandos[0].desplazamiento = auxInt;

        } else if (operandos[0].tipo == 3) { //si es de memoria

            aux = mv->memoria[ip];
            aux = aux & 0x0F;
            operandos[0].registro = aux;
            ip++;
            auxInt |= mv->memoria[ip];
            auxInt |= mv->memoria[ip + 1] & 0x00FF;
            operandos[0].desplazamiento = auxInt;

        }
    } else {
        operandos[0].registro = -1;
        operandos[0].desplazamiento = -1;
    } 
}
