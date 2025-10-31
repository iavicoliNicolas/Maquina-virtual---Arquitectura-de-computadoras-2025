#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/mv.h"
#include "../include/disassembler.h"
#include "../include/operando.h"
#include "../include/funciones.h"

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
    [0x0B] = "PUSH",
    [0x0C] = "POP",
    [0x0D] = "CALL",
    [0x0E] = "RET",
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
    [7]  = "SP",
    [8]  = "BP",
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
    [28] = "ES",
    [29] = "SS",
    [30] = "KS",
    [31] = "PS",
};
extern const char* mnemonicos[32];
extern const char* nombres_registros[32];


unsigned char r8(unsigned char *mem, int pos) {
    return (unsigned char)mem[pos];
}

unsigned short r16(unsigned char *mem, int pos) {
    // big-endian
    return (unsigned short)(((unsigned char)mem[pos] << 8) | (unsigned char)mem[pos+1]);
}
void imprimeOperando(operando op) {
    int sector,regid,size;
    unsigned char byte;
    const char *pref ;
    if (op.tipo == 0) //no hay operando
        return;

    if (op.tipo == 1) { // registro
        byte = (unsigned char)op.registro;
        sector = (byte >> 6) & 0x03;   // bits 7–6
        regid  = byte & 0x1F;          // bits 4–0

        const char *base = (regid < 32) ? nombres_registros[regid] : NULL;

        if (!base) {
            printf("R%d", regid);
            return;
        }

        switch (sector) {
            case 0:  printf("%s", base); break;      // registro completo (EAX)
            case 1:  printf("%cL", base[1]); break;  // byte bajo (AL)
            case 2:  printf("%cH", base[1]); break;  // byte alto (AH)
            case 3:  printf("%cX", base[1]); break;  // word (AX)
        }
    }
    else
        if (op.tipo == 2) { // inmediato
         printf("%d", op.desplazamiento);
        }
        else
           if (op.tipo == 3) { // memoria
             unsigned char byte = (unsigned char)op.registro;
             size = (byte >> 6) & 0x03;  // 00=l, 10=w, 11=b
             regid = byte & 0x1F;

             pref= "l"; // tamaño por defecto long
             if (size == 2)
                 pref = "w";
             else
                 if (size == 3)
                    pref = "b";

            const char *seg = (regid < 32) ? nombres_registros[regid] : NULL;

             printf("%s[", pref);
            if (seg)
               printf("%s", seg);
            else
               printf("R%d", regid);

            if (op.desplazamiento)
                printf("+%d", op.desplazamiento);
             printf("]");
            }

}
static int es_ascii_imprimible(unsigned char c) {
    return (c >= 32 && c <= 126);
}
void mostrarConstSegments(maquinaVirtual mv) {
    // porque tablaSegmentos[4] es CONST.
    const int SCONST = 4;

    int base = mv.tablaSegmentos[SCONST][0];
    int size = mv.tablaSegmentos[SCONST][1];
    if (size <= 0) return;

    int dir = base;
    int end = base + size;

    while (dir < end) {
        int start = dir;

        // 1) Intentar ASCII: secuencia de [isprint]+ '\0'
        int ascii_len = 0;
        while (start + ascii_len < end && es_ascii_imprimible(mv.memoria[start + ascii_len])) {
            ascii_len++;
        }
        if (ascii_len > 0 && start + ascii_len < end && mv.memoria[start + ascii_len] == 0x00) {
            int total = ascii_len + 1; // incluye '\0'

            // [addr] bytes (máx 7 con '..' si excede)
            printf("[%04X] ", start);
            int hex_show = (total > 7 ? 6 : total);
            for (int i = 0; i < hex_show; i++) printf("%02X ", mv.memoria[start + i]);
            if (total > 7) printf(".. ");
            else for (int i = hex_show; i < 7; i++) printf("   ");

            // | "texto..."
            printf("| \"");
            for (int i = 0; i < ascii_len; i++) {
                unsigned char c = mv.memoria[start + i];
                printf("%c", es_ascii_imprimible(c) ? c : '.');
            }
            printf("\"\n");

            dir = start + total;
            continue;
        }

        // 2) Intentar UTF-16LE: (c,0x00) ... (c,0x00)  +  \0
        // Requiere al menos 1 carácter + terminador.
        int ulen = 0; // bytes de pares (c,0)
        while (start + ulen + 1 < end &&
               es_ascii_imprimible(mv.memoria[start + ulen]) &&
               mv.memoria[start + ulen + 1] == 0x00) {
            ulen += 2;
        }
        if (ulen >= 2 && start + ulen + 1 < end &&
            mv.memoria[start + ulen] == 0x00 && mv.memoria[start + ulen + 1] == 0x00) {

            int total = ulen + 2; // incluye 0\

            printf("[%04X] ", start);
            int hex_show = (total > 7 ? 6 : total);
            for (int i = 0; i < hex_show; i++) printf("%02X ", mv.memoria[start + i]);
            if (total > 7) printf(".. ");
            else for (int i = hex_show; i < 7; i++) printf("   ");

            printf("| \"");
            for (int i = 0; i < ulen; i += 2) {
                unsigned char c = mv.memoria[start + i];         // byte bajo (ASCII)
                // mv.memoria[start + i + 1] es 0x00
                printf("%c", es_ascii_imprimible(c) ? c : '.');
            }
            printf("\"\n");

            dir = start + total;
            continue;
        }

        // 3) No es inicio de cadena válida -> avanzar 1 byte
        dir++;
    }
}






/* Retorna la dirección física del IP actual (con registro IP).
  IP contiene: 16 bits altos = índice en tabla de segmentos; 16 bits bajos = offset (desplazamiento en el seg)
*/
unsigned int ipFisicaDesdeRegistroIP(maquinaVirtual mv) {
    unsigned int ip_reg,seg_index,offset;
    int base,size;
    ip_reg = mv.registros[3]; // IP
    seg_index = (ip_reg >> 16) & 0xFFFF;
    offset = ip_reg & 0xFFFF;
    if (seg_index < 8) {
        base = mv.tablaSegmentos[seg_index][0];
        size = mv.tablaSegmentos[seg_index][1];
        return (unsigned int)(base + offset);
    }
    else {
        return 0xFFFFFFFF; // invalido
    }
}

void disassembler(maquinaVirtual mv) {
    int base_cs, size_cs, end_cs, ip;
    unsigned char b0;
    unsigned int reg_cs,seg_index,entry_fis;
    int tipoA, tipoB, codOp;
    operando opA = {0}, opB = {0};

    base_cs = mv.tablaSegmentos[0][0];
    size_cs = mv.tablaSegmentos[0][1];

     // Se usa para ver que CS no este inicializado en -1
    reg_cs = mv.registros[26];
    if (reg_cs != 0xFFFFFFFF) {
        seg_index = (reg_cs >> 16) & 0xFFFF;//Saco indice del segmento de parte alta de reg_cs
        if (seg_index < 8 && mv.tablaSegmentos[seg_index][1] > 0) { //que no se caiga de la tabla y que tamaño del seg>0
            base_cs = mv.tablaSegmentos[seg_index][0];
            size_cs = mv.tablaSegmentos[seg_index][1];
        }
    }
    end_cs  = base_cs + size_cs;
    //calculo punto de entrada (para marcar con'>')
    entry_fis = ipFisicaDesdeRegistroIP(mv);

    mostrarConstSegments(mv);

    ip = base_cs;
    while (ip < end_cs) {
        int direccion = ip;
        int start_ip  = ip;   // para calcular bytes de instrucción

        // primer byte de la instrucción
        b0 = r8(mv.memoria, ip++);
        tipoB = (b0 >> 6) & 0x03; // bits 7–6
        tipoA = (b0 >> 4) & 0x03; // bits 5–4
        codOp = b0 & 0x1F;

        // decodificar operandos en orden inverso
        ip += decodificaOperando(mv, ip, tipoB, &opB);
        ip += decodificaOperando(mv, ip, tipoA, &opA);

        int instr_len = ip - start_ip; // long total de instrucción

        // marca punto de entrada
        if ((unsigned int)direccion == entry_fis)
            printf(">");
        else
             printf(" ");

        printf("[%04X] ", direccion);

        // imprime instrucción en hex
        for (int i = 0; i < instr_len; i++) {
            printf("%02X ", (unsigned char)mv.memoria[start_ip + i]);
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
            printf("STOP ");
            break;
        }
        //------------>Ver si se puede omitir
         // limpiar operando para la próxima instrucción
        opA.tipo = opB.tipo = 0;
        opA.registro = opB.registro = 0;
        opA.desplazamiento = opB.desplazamiento = 0;
    }

    printf("\n");
}



int decodificaOperando(maquinaVirtual mv, int pos, int tipo, operando *op) {
    unsigned char byte;
    if (tipo == 0) { op->tipo = 0; return 0; }

    if (tipo == 1) { // Registro
        op->tipo = 1;
        byte = r8(mv.memoria, pos);
        op->registro = byte;
        return 1;
    }

    if (tipo == 2) { // Inmediato
        op->tipo = 2;
        op->desplazamiento = r16(mv.memoria, pos);
        return 2;
    }

    if (tipo == 3) { // Memoria
        op->tipo = 3;
        byte = r8(mv.memoria, pos);
        op->registro = byte & 0x1F;
        op->desplazamiento = r16(mv.memoria, pos+1);
        return 3;
    }

    return 0;
}







