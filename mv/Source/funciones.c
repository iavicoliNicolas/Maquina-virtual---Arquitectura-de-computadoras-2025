#include "funciones.h"
#include "operando.h"
#include "mv.h"
void cargaVF(Toperaciones *v){
    //operaciones de 2 operandos
    v[0x10] = MOV;
    v[0x11] = ADD;
    v[0x12] = SUB;
    v[0x13] = MUL;
    v[0x14] = DIV;
    v[0x15] = CMP;
    v[0x16] = SHL;
    v[0x17] = SHR;
    v[0x18] = SAR;
    v[0x19] = AND;
    v[0x1A] = OR;
    v[0x1B] = XOR;
    v[0x1C] = SWAP;
    v[0x1D] = LDL;
    v[0x1E] = LDH;
    v[0x1F] = RND;
    //operaciones de 1 operandos
    v[0x00] = SYS;
    v[0x01] = JMP;
    v[0x02] = JZ;
    v[0x03] = JP;
    v[0x04] = JN;
    v[0x05] = JNZ;
    v[0x06] = JNP;
    v[0x07] = JNN;
    v[0x08] = NOT;
    //operaciones sin operandos
    v[0x0F] = STOP;

}


void loadSYSOperationArray(funcionSys *vecLlamadas){
    vecLlamadas[0] = readSys;
    vecLlamadas[1] = writeSys; 
}

//funciones assembler
void MOV(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[0]));
}
void ADD(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) + getOp(mv, op[0]));
}
void SUB(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) - getOp(mv, op[0]));
}
void MUL(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) * getOp(mv, op[0]));
}
void DIV(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) / getOp(mv, op[0]));
}
void CMP(maquinaVirtual *mv, operando *op) {}
void SHL(maquinaVirtual *mv, operando *op) {}
void SHR(maquinaVirtual *mv, operando *op) {}
void SAR(maquinaVirtual *mv, operando *op) {}
void AND(maquinaVirtual *mv, operando *op) {}
void OR(maquinaVirtual *mv, operando *op) {}
void XOR(maquinaVirtual *mv, operando *op) {}
void SWAP(maquinaVirtual *mv, operando *op) {}
void LDL(maquinaVirtual *mv, operando *op) {}
void LDH(maquinaVirtual *mv, operando *op) {}
void RND(maquinaVirtual *mv, operando *op) {}
void SYS(maquinaVirtual *mv, operando *op) {}
void JMP(maquinaVirtual *mv, operando *op) {}
void JZ(maquinaVirtual *mv, operando *op) {}
void JP(maquinaVirtual *mv, operando *op) {}
void JN(maquinaVirtual *mv, operando *op) {}
void JNZ(maquinaVirtual *mv, operando *op) {}
void JNP(maquinaVirtual *mv, operando *op) {}
void JNN(maquinaVirtual *mv, operando *op) {}
void NOT(maquinaVirtual *mv, operando *op) {}
void STOP(maquinaVirtual *mv, operando *op) {}

void readSys(maquinaVirtual *mv, int aux) {}
void writeSys(maquinaVirtual *mv, int aux) {}
void ejecutarOperacion(maquinaVirtual *mv, int opcode, operando *op) {
   
}