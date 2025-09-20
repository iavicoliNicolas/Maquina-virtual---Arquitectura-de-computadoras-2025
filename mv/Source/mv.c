#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mv.h"

int corrigeSize(int size)
{
    int aux1 = 0, aux2 = 0;
    aux1 = (size >> 8) & 0x00FF;
    aux2 = (size & 0x00FF) << 8;
    return aux2 | aux1;
}

int puntero(int posFisica) {
}
int logicoAFisico();

void setReg(maquinaVirtual *mv, int reg, int valor) {
    mv->registros[reg] = valor;
}
int getReg(maquinaVirtual *mv, int reg) {
    return mv->registros[reg];
}

void leerMV(maquinaVirtual *mv, FILE* arch) {

    char cabecera[5];
    unsigned char version;
    unsigned short int tamano_codigo;

    // 1. Leer cabecera del archivo VMX
    fread(cabecera, sizeof(char), 5, arch);
    cabecera[5] = '\0'; // Null-terminate para comparación

    // Verificar identificador "VMX25"
    if (strcmp(cabecera, "VMX25") != 0) {
        fprintf(stderr, "Error: Formato de archivo invalido\n");
        exit(EXIT_FAILURE);
    }

    // 2. Leer versión
    fread(&version, sizeof(unsigned char), 1, arch);
    if (version != 1) {
        fprintf(stderr, "Error: Versión no soportada: %d\n", version);
        exit(EXIT_FAILURE);
    }

    // 3. Leer tamaño del código (2 bytes, little-endian)
    fread(&tamano_codigo, sizeof(unsigned short int), 1, arch);
    tamano_codigo = corrigeSize(tamano_codigo);

    // 4. Verificar que el código cabe en memoria
    if (tamano_codigo > 16384) {
        fprintf(stderr, "Error: Código demasiado grande (%d bytes)\n", tamano_codigo);
        exit(EXIT_FAILURE);
    }

    // 5. Cargar código en memoria (segmento de código)
    fread(mv->memoria, sizeof(char), tamano_codigo, arch);

    // 6. Inicializar tabla de descriptores de segmentos
    // Entrada 0: Segmento de código
    mv->tablaSegmentos[0][0] = 0;                      // Base = 0
    mv->tablaSegmentos[0][1] = tamano_codigo - 1;      // Tamaño del código

    // Entrada 1: Segmento de datos
    mv->tablaSegmentos[1][0] = tamano_codigo;          // Base = fin del código
    mv->tablaSegmentos[1][1] = 16384 - tamano_codigo;  // Tamaño de datos

    // Las demás entradas (2-7) se inicializan a 0
    for (int i = 2; i < 8; i++) {
        for (int j = 0; j < 2; j++)
            mv->tablaSegmentos[i][j] = 0;
    }

    // 7. Inicializar registros
    for (int i = 0; i < 32; i++) {
        mv->registros[i] = 0;
    }

    // Inicializar registros especiales
    mv->registros[26] = 0x00000000;  // CS: segmento de código (tabla entrada 0)
    mv->registros[27] = 0x00010000;  // DS: segmento de datos (tabla entrada 1)
    mv->registros[3]  = 0x00000000;  // IP: comienza en inicio del código
    mv->registros[17] = 0;           // Condition Code inicial

    printf("Programa cargado: %d bytes de código\n", tamano_codigo);
}

/*
void muestraCS(maquinaVirtual mv) {
    printf("=== SEGMENTO DE CÓDIGO (CS) ===\n");

    // Obtener información del segmento de código desde tabla de segmentos
    int base_cs = mv.tablaSegmentos[0][0];      // Base del segmento de código
    int tamano_cs = mv.tablaSegmentos[0][1];    // Tamaño del segmento de código

    printf("Base: 0x%04X, Tamaño: %d bytes\n", base_cs, tamano_cs);
    printf("Dirección IP: 0x%04X\n", mv.registros[3]); // IP
    printf("\n");

    // Mostrar contenido en formato hexadecimal y ASCII
    printf("Direc.   Hexadecimal                         ASCII\n");
    printf("-------  ----------------------------------  --------\n");

    for (int i = 0; i < tamano_cs; i += 16) {
        printf("0x%04X:  ", base_cs + i);

        for (int j = 0; j < 16; j++) {
            if (i + j < tamano_cs) {
                printf("%02X ", (unsigned char)mv.memoria[base_cs + i + j]);
            } else {
                printf("   ");
            }
            if (j == 7) printf(" ");
        }

        printf(" ");

        for (int j = 0; j < 16; j++) {
            if (i + j < tamano_cs) {
                unsigned char c = mv.memoria[base_cs + i + j];
                if (c >= 32 && c <= 126) {
                    printf("%c", c);
                } else {
                    printf(".");
                }
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }

    printf("\nRegistro CS: 0x%08X\n", mv.registros[26]);
}
*/

void ejecutarMV(maquinaVirtual *mv) {

    operando op[2];
    Toperaciones v[32];
    cargaVF(v);

    funcionSys vecLlamadas[2];
    loadSYSOperationArray(vecLlamadas);

    while (mv->registros[OPC] != 0x0F || mv->registros[IP] < 16384) {

        leerInstruccion(*mv, op);
        setReg(mv, OPC, getMem(mv, op[0]));
        setReg(mv, OP1, getMem(mv, op[1]));
        setReg(mv, OP2, getMem(mv, op[2]));
        setReg(mv, IP, mv->registros[IP] + 1 + (op[0].tipo != 0) + (op[1].tipo != 0) + (op[2].tipo != 0));
        ejecutarOperacion(mv, mv->registros[OPC], op);
    }
    printf("\nEjecución finalizada\n");
}

void disassembler(maquinaVirtual mv) {
    
}