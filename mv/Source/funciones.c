#include "funciones.h"

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

void setLAR(maquinaVirtual *mv, int valor) {
    mv->registros[LAR] = valor;
}
void setMAR(maquinaVirtual *mv, int valor) {
    mv->registros[MAR] = valor;
}
void setMBR(maquinaVirtual *mv, int valor) {
    mv->registros[MBR] = valor;
}
void setOP1(maquinaVirtual *mv, operando valor, int tipo) {
    
}
void setOP2(maquinaVirtual *mv, operando valor, int tipo) {
}

void setCC(maquinaVirtual *mv, int resultado) {
    if (resultado == 0) {
        mv->registros[CC] = 0; // Cero
    } else if (resultado > 0) {
        mv->registros[CC] = 1; // Positivo
    } else {
        mv->registros[CC] = -1; // Negativo
    }
}


//funciones assembler
void MOV(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[0]));
}
void ADD(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) + getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SUB(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) - getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void MUL(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) * getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void DIV(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) / getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void CMP(maquinaVirtual *mv, operando *op){
    int resultado = getOp(mv, op[1]) - getOp(mv, op[0]);
    setCC(mv, resultado);
}
void SHL(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) << getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SAR(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) >> getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SHR(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], (unsigned int)getOp(mv, op[1]) >> getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void AND(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) & getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void OR(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) | getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void XOR(maquinaVirtual *mv, operando *op){
    setOp(mv, op[1], getOp(mv, op[1]) ^ getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SWAP(maquinaVirtual *mv, operando *op){
    int temp = getOp(mv, op[0]);
    setOp(mv, op[0], getOp(mv, op[1]));
    setOp(mv, op[1], temp);
}
void RND(maquinaVirtual *mv, operando *op){
    setOp(mv, op[0], rand());
}
void SYS(maquinaVirtual *mv, operando *op){
    Sistema aux = getOp(mv, op[0]);
    funcionSys vecLlamadas[2];
    loadSYSOperationArray(vecLlamadas);
    if (aux >= 0 && aux <= 1) {
        vecLlamadas[aux](mv, aux);
    } else {
        fprintf(stderr, "Error: Llamada al sistema no valida: %d\n", aux);
        exit(EXIT_FAILURE);
    }
}
void JMP(maquinaVirtual *mv, operando *op){
    mv->registros[IP] = getOp(mv, op[0]);
}
void JZ(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] == 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JP(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] > 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JN(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] < 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNZ(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] != 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNP(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] >= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNN(maquinaVirtual *mv, operando *op){
    if (mv->registros[CC] <= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void LDL(maquinaVirtual *mv, operando *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFFFF00) | (getOp(mv, op[1]) & 0x000000FF));
}
void LDH(maquinaVirtual *mv, operando *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFF00FF) | ((getOp(mv, op[1]) & 0x000000FF) << 8));
}
void NOT(maquinaVirtual *mv, operando *op){
    setOp(mv, op[0], ~getOp(mv, op[0]));
}
void STOP(maquinaVirtual *mv, operando *op){
    exit(EXIT_SUCCESS);
}