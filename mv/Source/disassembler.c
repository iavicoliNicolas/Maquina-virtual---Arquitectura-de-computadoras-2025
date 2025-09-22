#include <stdio.h>
#include <string.h>
#include "mv.h"
#include "disassembler.h"
#include "operando.h"
#include "funciones.h"
const char* mnemonicos[32] = {
    [0x00] = "SYS",
    [0x01] = "JMP",
    [0x02] = "JZ",
    [0x03] = "JP",
    [0x04] = "JN",
    [0x05] = "JNZ",
    [0x06] = "JNP",
    [0x07] = "JNN",
    [0x08] = "NOT",
    [0x0F] = "STOP",
    [0x10] = "MOV",
    [0x11] = "ADD",
    [0x12] = "SUB",
    [0x13] = "MUL",
    [0x14] = "DIV",
    [0x15] = "CMP",
    [0x16] = "SHL",
    [0x17] = "SHR",
    [0x18] = "SAR",
    [0x19] = "AND",
    [0x1A] = "OR",
    [0x1B] = "XOR",
    [0x1C] = "SWAP",
    [0x1D] = "LDL",
    [0x1E] = "LDH",
    [0x1F] = "RND"
};

const char* nombres_registros[32] = {
    [0]  = "LAR",   
    [1]  = "MAR",   
    [2]  = "MBR",  
    [3]  = "IP",    
    [4]  = "OPC",   
    [5]  = "OP1",   
    [6]  = "OP2",   
    [7]  = NULL,    
    [8]  = NULL,    
    [9]  = NULL,   
    [10] = "EAX",   
    [11] = "EBX",  
    [12] = "ECX",  
    [13] = "EDX", 
    [14] = "EEX",   
    [15] = "EFX",   
    [16] = "AC",    
    [17] = "CC",    
    [18] = NULL,    
    [19] = NULL,    
    [20] = NULL,
    [21] = NULL,
    [22] = NULL,
    [23] = NULL,
    [24] = NULL,
    [25] = NULL,
    [26] = "CS",    
    [27] = "DS",    
    [28] = NULL,
    [29] = NULL,
    [30] = NULL,
    [31] = NULL
};
extern const char* mnemonicos[32];
extern const char* nombres_registros[32];

// === helpers ===
static inline unsigned char r8(char *mem, int pos) {
    return (unsigned char)mem[pos];
}

static inline unsigned short r16(char *mem, int pos) {
    // big-endian
    return (unsigned short)(((unsigned char)mem[pos] << 8) | (unsigned char)mem[pos+1]);
}


void disassembler(maquinaVirtual *mv) {
    int base_cs, size_cs, end_cs, ip;
    unsigned char b0;
    int tipoA, tipoB, codOp;
    operando opA = {0}, opB = {0};

    base_cs = mv->tablaSegmentos[0][0];
    size_cs = mv->tablaSegmentos[0][1];
    end_cs  = base_cs + size_cs;

    ip = base_cs;
    while (ip < end_cs) {
        int direccion = ip;
        int start_ip  = ip;   // para calcular bytes de instrucción

        // primer byte de la instrucción
        b0 = r8(mv->memoria, ip++);
        tipoB = (b0 >> 6) & 0x03; // bits 7–6
        tipoA = (b0 >> 4) & 0x03; // bits 5–4
        codOp = b0 & 0x1F;

        // decodificar operandos en orden inverso
        ip += decodificaOperando(mv, ip, tipoB, &opB);
        ip += decodificaOperando(mv, ip, tipoA, &opA);

        int instr_len = ip - start_ip; // long total de instrucción

      
        printf("[%04X] ", direccion);

        // imprime instrucción en hex
        for (int i = 0; i < instr_len; i++) {
            printf("%02X ", (unsigned char)mv->memoria[start_ip + i]);
        }
        // relleno para alinear 
        for (int i = instr_len; i < 8; i++) {
            printf("   ");
        }
        printf(" | ");
        // imprime mnemónico
        if (mnemonicos[codOp])
            printf(" %s ", mnemonicos[codOp]);
        else
            printf("(OPC?? %02X) ", codOp);

        // imprime operandos
        if (opA.tipo != 0) {
            imprimeOperando(opA);
            if (opB.tipo != 0) {
                printf(" , ");
                imprimeOperando(opB);
            }
        } else if (opB.tipo != 0) {
            imprimeOperando(opB);
        }

        printf(";\n");

        // si es STOP termina
        if (codOp == 0x0F) {
            break;
        }
    }

    printf("\n");
}



int decodificaOperando(maquinaVirtual *mv, int pos, int tipo, operando *op) {
    unsigned char byte;
    if (tipo == 0) { op->tipo = 0; return 0; }

    if (tipo == 1) { // Registro
        op->tipo = 1;
        byte = r8(mv->memoria, pos);
        op->registro = byte & 0x1F;
        return 1;
    }

    if (tipo == 2) { // Inmediato
        op->tipo = 2;
        op->desplazamiento = r16(mv->memoria, pos);
        return 2;
    }

    if (tipo == 3) { // Memoria
        op->tipo = 3;
        byte = r8(mv->memoria, pos);
        op->registro = byte & 0x1F;
        op->desplazamiento = r16(mv->memoria, pos+1);
        return 3;
    }

    return 0;
}







