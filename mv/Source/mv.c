#include "mv.h"

int corrigeSize(int size)
{
    int aux1 = 0, aux2 = 0;
    aux1 = (size >> 8) & 0x00FF;
    aux2 = (size & 0x00FF) << 8;
    return aux2 | aux1;
}
void iniciaNombresOperaciones( CodOperacion nombresOperaciones[32] ) {

    CodOperacion v[32] = {
        "SYS",   // 0x00
        "JMP",   // 0x01
        "JZ",    // 0x02
        "JP",    // 0x03
        "JN",    // 0x04
        "JNZ",   // 0x05
        "JNP",   // 0x06
        "JNN",   // 0x07
        "NOT",   // 0x08
        "",      // 0x09 (No definido)
        "",      // 0x0A (No definido)
        "",      // 0x0B (No definido)
        "",      // 0x0C (No definido)
        "",      // 0x0D (No definido)
        "",      // 0x0E (No definido)
        "STOP",  // 0x0F
        "MOV",   // 0x10
        "ADD",   // 0x11
        "SUB",   // 0x12
        "MUL",   // 0x13
        "DIV",   // 0x14
        "CMP",   // 0x15
        "SHL",   // 0x16
        "SHR",   // 0x17
        "SAR",   // 0x18
        "AND",   // 0x19
        "OR",    // 0x1A
        "XOR",   // 0x1B
        "SWAP",  // 0x1C
        "LDL",   // 0x1D
        "LDH",   // 0x1E
        "RND"    // 0x1F
    };
    nombresOperaciones = v;
}
void iniciaRegNombres( CodOperacion nombresRegistros[32][4] ) {
    CodOperacion v = {
        {"LAR"," "," "," "},  // 0
        {"MAR"," "," "," "},  // 1
        {"MBR"," "," "," "},  // 2
        {"IP"," "," "," "},   // 3
        {"OPC"," "," "," "},  // 4
        {"OP1"," "," "," "},  // 5
        {"OP2"," "," "," "},  // 6
        {""," "," "," "},     // 7 (Reservado)
        {""," "," "," "},     // 8
        {""," "," "," "},     // 9
        {"EAX", "AL", "AH", "AX"},  // 10
        {"EBX", "BL", "BH", "BX"},  // 11
        {"ECX", "CL", "CH", "CX"},  // 12
        {"EDX", "DL", "DH", "DX"},  // 13
        {"EEX", "EL", "EH", "EX"},  // 14
        {"EFX", "FL", "FH", "FX"},  // 15
        {"AC"," "," "," "},         // 16
        {"CC"," "," "," "},         // 17
        {""," "," "," "},           // 18
        {""," "," "," "},           // 19
        {""," "," "," "},           // 20
        {""," "," "," "},           // 21 (Reservado)
        {""," "," "," "},           // 22
        {""," "," "," "},           // 23
        {""," "," "," "},           // 24
        {""," "," "," "},           // 25
        {"CS"," "," "," "},         // 26
        {"DS"," "," "," "},         // 27
        {""," "," "," "},           // 28 (Reservado)
        {""," "," "," "},           // 29
        {""," "," "," "},           // 30
        {""," "," "," "},           // 31
    };
}
void muestRegi(char muest[], char *NombreRegistro[][4])
{
    char seg = (muest[0] >> 4) & 0x03;
    char reg = muest[0] & 0x0F;

    printf("%s", NombreRegistro[reg][seg]);
}
void muestMem(char muest[], char *NomReg[][4])
{
    char reg = muest[0] & 0x0F;
    char tam = (muest[0] >> 6) & 0x3;
    short int offset = (muest[1] << 8) | muest[2];
    if (tam == 0x3)
        printf("b");
    else if (tam == 0x1)
        printf("w");
    else
        printf("l");
    printf("[%s", NomReg[reg][0]);
    if (offset != 0)
    {
        if (offset < 0)
            printf("-%d", abs(offset));
        else
            printf("+%d", offset);
    }
    printf("]");
}
void muestInme(char muest[])
{
    int num = ((muest[0] << 8) | (muest[1] & 0xff));

    printf("%d", num);
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
    mv->tablaSegmentos[0][0] = 0;                    // Base = 0
    mv->tablaSegmentos[0][1] = tamano_codigo - 1;        // Tamaño del código

    // Entrada 1: Segmento de datos
    mv->tablaSegmentos[1][0] = tamano_codigo;        // Base = fin del código
    mv->tablaSegmentos[1][1] = 16384 - tamano_codigo; // Tamaño del datos

    // Las demás entradas (2-7) se inicializan a 0
    for (int i = 2; i < 8; i++) {
        for ( int j = 0; i < 2; j++)
            mv->tablaSegmentos[i][j] = 0;
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
/*
void muestraCS(maquinaVirtual mv) {
    printf("=== SEGMENTO DE CÓDIGO (CS) ===\n");

    // Obtener información del segmento de código desde tabla de segmentos
    int base_cs = mv.tablaSegmentos[0][0];      // Base del segmento de código
    int tamano_cs = mv.tablaSegmentos[0][1];    // Tamaño del segmento de código

    printf("Base: 0x%04X, Tamano: %d bytes\n", base_cs, tamano_cs);
    printf("Dirección IP: 0x%04X\n", mv.registros[3]); // IP
    printf("\n");

    // Mostrar contenido en formato hexadecimal y ASCII
    printf("Direc.   Hexadecimal                         ASCII\n");
    printf("-------  ----------------------------------  --------\n");

    for (int i = 0; i < tamano_cs; i += 16) {
        // Dirección actual
        printf("0x%04X:  ", base_cs + i);

        // Bytes en hexadecimal
        for (int j = 0; j < 16; j++) {
            if (i + j < tamano_cs) {
                printf("%02X ", (unsigned char)mv.memoria[base_cs + i + j]);
            } else {
                printf("   "); // Espacios para alinear
            }

            if (j == 7) printf(" "); // Separador a mitad de línea
        }

        printf(" ");

        // Bytes en ASCII (solo caracteres imprimibles)
        for (int j = 0; j < 16; j++) {
            if (i + j < tamano_cs) {
                unsigned char c = mv.memoria[base_cs + i + j];
                if (c >= 32 && c <= 126) { // Caracteres imprimibles
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

    // Mostrar también el valor del registro CS
    printf("\nRegistro CS: 0x%08X\n", mv.registros[26]);
}
*/

void ejecutarMV(maquinaVirtual *mv);

void disassembler( maquinaVirtual mv ) {

    int IPaux, i, j, segCS, offsetCS, dirFisCS;
    unsigned char inst, OP1, OP2, Cod;
    char muestra1[3], muestra2[3];
    segCS = mv.registros[CS] >> 16;
    offsetCS = mv.registros[CS] & 0x0000FFFF;
    IPaux = mv.tablaSegmentos[segCS][0] + offsetCS;

    char *nombresRegistros[32][4];
    iniciaRegNombres(nombresRegistros);

    char *nombresOperaciones[32];
    iniciaNombresOperaciones(nombresOperaciones);

    printf("\n\nInicio Disassembler\n\n");

    while (IPaux < (mv.tablaSegmentos[segCS][0] + mv.tablaSegmentos[segCS][1]))
    {
        inst = mv.memoria[IPaux];
        Cod = inst & 0b00011111;
        OP1 = (inst >> 4) & 0x03;
        OP2 = (inst >> 6) & 0x03;

        if (IPaux == (mv.tablaSegmentos[mv.memoria[CS] >> 16][0]) )
            printf(">[%04X] %02X ", IPaux, (unsigned)inst);
        else
            printf(" [%04X] %02X ", IPaux, (unsigned)inst);

        for (i = 0; i < ((~OP2) & 0x03); i++)
        {
            printf("%02X ", mv.memoria[IPaux + 1 + i]);
            muestra2[i] = mv.memoria[IPaux + 1 + i];
        }

        j = 0;
        for (i = ((~OP2) & 0x03); i < (((~OP1) & 0x03) + ((~OP2) & 0x03)); i++)
        {
            printf("%02X ", mv.memoria[IPaux + 1 + i]);
            muestra1[j] = mv.memoria[IPaux + 1 + i];
            j++;
        }

        for (i = (((~OP1) & 0x03) + ((~OP2) & 0x03)); i < 6; i++)
            printf("   ");
        printf("| %s ", nombresOperaciones[Cod]);

        if (OP1 != 3)
        {
            switch (OP1)
            {
            case 0:
                muestMem(muestra1, nombresRegistros);
                break;
            case 1:
                muestInme(muestra1);
                break;
            case 2:
                muestRegi(muestra1, nombresRegistros);
                break;
            }
            if (OP2 != 3)
            {
                printf(",");
                switch (OP2)
                {
                case 0:
                    muestMem(muestra2, nombresRegistros);
                    break;
                case 1:
                    muestInme(muestra2);
                    break;
                case 2:
                    muestRegi(muestra2, nombresRegistros);
                    break;
                }
            }
        }
        else
            switch (OP2)
            {
            case 0:
                muestMem(muestra2, nombresRegistros);
                break;
            case 1:
                muestInme(muestra2);
                break;
            case 2:
                muestRegi(muestra2, nombresRegistros);
                break;
            }
        printf("\n");
        IPaux += 1 + ((~OP1) & 0x03) + ((~OP2) & 0x03);
    }
    printf("\n\nFin Disassembler\n\n");
}
