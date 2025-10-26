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
int segmentoEsProbableConst(unsigned char *mem, int base, int size) {
    int i, end;
    end = base + size;
    printf("Entra  2\n");
    for (i = base; i + 5 < end; i += 2) { // mínimo 3 caracteres = 6 bytes
        int count = 0;
        while (i + count + 1 < end && isprint(mem[i + count]) && mem[i + count + 1] == 0)
            count += 2;

        if (count >= 6) // 3 o más caracteres consecutivos
            return 1;
    }

    return 0; // no hay cadena de ≥3 caracteres
}
void mostrarConstSegments(maquinaVirtual mv) {
    int base, size, s, dir, end, i;
    unsigned char c;
    printf("  Dump de primeros bytes del segmento %d:\n  ", s);
    for (int k = 0; k < 32 && k < size; k++) {
         printf("%02X ", mv.memoria[base + k]);
    }
printf("\n");
    printf("==> Entró a mostrarConstSegments()\n");

    for (s = 0; s < 8; s++) {
        base = mv.tablaSegmentos[s][0];
        size = mv.tablaSegmentos[s][1];
        printf("Segmento %d: base=%d size=%d\n", s, base, size);

        if (size <= 0) {
            printf("  Segmento %d vacío, se salta.\n", s);
            continue;
        }

        dir = base;
        end = base + size;

        printf("  Analizando desde %d hasta %d\n", dir, end);

        while (dir + 5 < end) { // mínimo 3 caracteres UTF-16LE
            int start = dir;
            int strlen_bytes = 0;

            // contar caracteres consecutivos UTF-16LE imprimibles
            while (dir + strlen_bytes + 1 < end &&
                   isprint(mv.memoria[dir + strlen_bytes]) &&
                   mv.memoria[dir + strlen_bytes + 1] == 0)
            {
                strlen_bytes += 2;
            }

            if (strlen_bytes > 0)
                printf("  Pos %d: encontró posible texto UTF16 de %d bytes\n", start, strlen_bytes);

            // ahora verificar que haya terminador 0x00 0x00 inmediatamente después
            if (strlen_bytes >= 6 && dir + strlen_bytes + 1 < end &&
                mv.memoria[dir + strlen_bytes] == 0 &&
                mv.memoria[dir + strlen_bytes + 1] == 0)
            {
                printf("  Cadena válida detectada en %d (len=%d)\n", start, strlen_bytes);

                strlen_bytes += 2; // incluir terminador

                // imprimir dirección y HEX limitado a 7 bytes
                printf("[%04X] ", start);
                int hex_show = (strlen_bytes > 7 ? 7 : strlen_bytes);
                for (i = 0; i < hex_show; i++)
                    printf("%02X ", mv.memoria[start + i]);
                if (strlen_bytes > 7)
                    printf(".. ");
                else
                    for (i = hex_show; i < 7; i++)
                        printf("   ");

                // imprimir ASCII completa
                printf(" | \"");
                for (i = 0; i < strlen_bytes - 2; i += 2) // -2 para no imprimir terminador
                    printf("%c", mv.memoria[start + i]);
                printf("\"\n");

                dir += strlen_bytes; // avanzar al siguiente bloque
            } else {
                if (strlen_bytes > 0)
                    printf("  En %d había %d bytes imprimibles pero no terminador UTF16.\n",
                           start, strlen_bytes);
                dir += 2; // no es cadena válida, avanzar
            }
        }
    }

    printf("<== Sale de mostrarConstSegments()\n");
}

/*void mostrarConstSegments(maquinaVirtual mv) {
    int base, size, s, dir, end, i;
    unsigned char c;
    printf("Entraaaaaa\n");
    for (s = 0; s < 8; s++) {
        base = mv.tablaSegmentos[s][0];
        size = mv.tablaSegmentos[s][1];
        if (size <= 0) continue;

        dir = base;
        end = base + size;

        while (dir + 5 < end) { // mínimo 3 caracteres UTF-16LE
            int start = dir;
            int strlen_bytes = 0;

            // contar caracteres consecutivos UTF-16LE imprimibles
            while (dir + strlen_bytes + 1 < end &&
                   isprint(mv.memoria[dir + strlen_bytes]) &&
                   mv.memoria[dir + strlen_bytes + 1] == 0)
            {
                strlen_bytes += 2;
            }

            // ahora verificar que haya terminador 0x00 0x00 inmediatamente después
            if (strlen_bytes >= 6 && dir + strlen_bytes + 1 < end &&
                mv.memoria[dir + strlen_bytes] == 0 &&
                mv.memoria[dir + strlen_bytes + 1] == 0)
            {
                strlen_bytes += 2; // incluir terminador

                // imprimir dirección y HEX limitado a 7 bytes
                printf("[%04X] ", start);
                int hex_show = (strlen_bytes > 7 ? 7 : strlen_bytes);
                for (i = 0; i < hex_show; i++)
                    printf("%02X ", mv.memoria[start + i]);
                if (strlen_bytes > 7)
                    printf(".. ");
                else
                    for (i = hex_show; i < 7; i++)
                        printf("   ");

                // imprimir ASCII completa
                printf(" | \"");
                for (i = 0; i < strlen_bytes - 2; i += 2) // -2 para no imprimir terminador
                    printf("%c", mv.memoria[start + i]);
                printf("\"\n");

                dir += strlen_bytes; // avanzar al siguiente bloque
            } else {
                dir += 2; // no es cadena válida, avanzar al siguiente UTF-16LE
            }
        }
    }
}*/



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







