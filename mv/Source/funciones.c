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

void setLAR(maquinaVirtual *mv, int segmento, int desplaz) {
    mv->registros[LAR] = ((segmento & 0xFFFF) << 16) | (desplaz & 0xFFFF);
}
void setMAR(maquinaVirtual *mv, int nbytes, int dirFisica) {
    mv->registros[MAR] = ((nbytes & 0xFFFF) << 16) | (dirFisica & 0xFFFF);
}
void setMBR(maquinaVirtual *mv, int valor) {
    mv->registros[MBR] = valor;
}
void setRegOP(maquinaVirtual *mv, int reg, operando valor, int tipo) {

    mv->registros[reg] = tipo << 24; //almacenar el tipo en los 8 bits mas significativos

    switch (tipo) {
        case 0: //no usado
            mv->registros[reg] |= 0;
            break;
        case 1: //registro
            mv->registros[reg] |= (valor.registro & 0xFF);
            break;
        case 2: //inmediato
            mv->registros[reg] |= (valor.desplazamiento & 0xFFFF);
            break;
        case 3: //memoria
            mv->registros[reg] |= (valor.registro & 0xFF) << 16;
            mv->registros[reg] |= (valor.desplazamiento & 0xFFFF);
            break;
        default:
            fprintf(stderr, "Error: Tipo de operando invalido: %d\n", tipo);
            exit(EXIT_FAILURE);
    }
}

void setCC(maquinaVirtual *mv, int resultado) {
    if (resultado == 0) {
        mv->registros[CC] = 0b0100000000000000; // Cero
    } else if (resultado > 0) {
        mv->registros[CC] = 0b0000000000000000; // Positivo
    } else {
        mv->registros[CC] = 0b1000000000000000; // Negativo
    }
}


//funciones assembler
void MOV(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[0]));
}
void ADD(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) + getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SUB(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) - getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void MUL(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) * getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void DIV(maquinaVirtual *mv, int *op) {
    int aux = getOp(mv, op[1]);
    if ( aux == 0) {
        fprintf(stderr, "Error: Division por cero\n");
        exit(EXIT_FAILURE);
    }
    setOp(mv, op[1], getOp(mv, op[1]) / getOp(mv, op[0]));
    mv->registros[AC] = getOp(mv, op[1]) % getOp(mv, op[0]); //guardar el resto en AC
    setCC(mv, getOp(mv, op[1]));
}

void CMP(maquinaVirtual *mv, int *op){
    int resultado = getOp(mv, op[1]) - getOp(mv, op[0]);
    setCC(mv, resultado);
}
void SHL(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) << getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SAR(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) >> getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SHR(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], (unsigned int)getOp(mv, op[1]) >> getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void AND(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) & getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void OR(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) | getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void XOR(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) ^ getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SWAP(maquinaVirtual *mv, int *op){
    int temp = getOp(mv, op[0]);
    setOp(mv, op[0], getOp(mv, op[1]));
    setOp(mv, op[1], temp);
}
void RND(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], rand());
}
void SYS(maquinaVirtual *mv, int     *op){
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
void JMP(maquinaVirtual *mv, int *op){
    mv->registros[IP] = getOp(mv, op[0]);
}
void JZ(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] == 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JP(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] > 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JN(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] < 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNZ(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] != 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNP(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] >= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNN(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] <= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void LDL(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFFFF00) | (getOp(mv, op[1]) & 0x000000FF));
}
void LDH(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFF00FF) | ((getOp(mv, op[1]) & 0x000000FF) << 8));
}
void NOT(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], ~getOp(mv, op[0]));
}
void STOP(maquinaVirtual *mv, int *op){
    exit(EXIT_SUCCESS);
}
void readSys(maquinaVirtual *mv, int arg) {
   
}

void writeSys(maquinaVirtual *mv, int arg) {
}