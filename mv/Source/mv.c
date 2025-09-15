#include <stdio.h>
#include <stdlib.h>
#include "mv.h"


void leerMV(maquinaVirtual *mv, FILE* arch) {

    char cabecera[5];
    unsigned char version;
    unsigned short tamano_codigo;

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
    fread(&tamano_codigo, sizeof(unsigned short), 1, arch);

    // 4. Verificar que el código cabe en memoria
    if (tamano_codigo > 16384) {
        fprintf(stderr, "Error: Código demasiado grande (%d bytes)\n", tamano_codigo);
        exit(EXIT_FAILURE);
    }

    // 5. Cargar código en memoria (segmento de código)
    fread(mv->memoria, sizeof(char), tamano_codigo, arch);

    // 6. Inicializar tabla de descriptores de segmentos
    // Entrada 0: Segmento de código
    mv->tablaSegmentos[0][0] = 0;                    // Base = 0
    mv->tablaSegmentos[0][1] = tamano_codigo - 1;        // Tamaño del código

    // Entrada 1: Segmento de datos
    mv->tablaSegmentos[1][0] = tamano_codigo;        // Base = fin del código
    mv->tablaSegmentos[1][1] = 16384 - tamano_codigo; // Tamaño del datos

    // Las demás entradas (2-7) se inicializan a 0
    for (int i = 4; i < 8; i++) {
        mv->tablaSegmentos[i] = 0;
    }

    // 7. Inicializar registros
    for (int i = 0; i < 32; i++) {
        mv->registros[i] = 0;
    }

    // Inicializar registros especiales
    mv->registros[26] = 0x00000000;  // CS: segmento de código (tabla entrada 0)
    mv->registros[27] = 0x00010000;  // DS: segmento de datos (tabla entrada 1)
    mv->registros[3] = 0x00000000;   // IP: comienza en inicio del código
    mv->registros[17] = 0;                      // Condition Code inicial

    printf("Programa cargado: %d bytes de codigo\n", tamano_codigo);
}
void ejecutarMV(maquinaVirtual *mv);

void disassembler( maquinaVirtual mv );
