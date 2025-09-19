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
    cabecera[5] = '\0'; // Null-terminate para comparaci�n

    // Verificar identificador "VMX25"
    if (strcmp(cabecera, "VMX25") != 0) {
        fprintf(stderr, "Error: Formato de archivo invalido\n");
        exit(EXIT_FAILURE);
    }

    // 2. Leer versi�n
    fread(&version, sizeof(unsigned char), 1, arch);
    if (version != 1) {
        fprintf(stderr, "Error: Versi�n no soportada: %d\n", version);
        exit(EXIT_FAILURE);
    }

    // 3. Leer tama�o del c�digo (2 bytes, little-endian)
    fread(&tamano_codigo, sizeof(unsigned short int), 1, arch);
    tamano_codigo = corrigeSize(tamano_codigo);

    // 4. Verificar que el c�digo cabe en memoria
    if (tamano_codigo > 16384) {
        fprintf(stderr, "Error: C�digo demasiado grande (%d bytes)\n", tamano_codigo);
        exit(EXIT_FAILURE);
    }

    // 5. Cargar c�digo en memoria (segmento de c�digo)
    fread(mv->memoria, sizeof(char), tamano_codigo, arch);

    // 6. Inicializar tabla de descriptores de segmentos
    // Entrada 0: Segmento de c�digo
    mv->tablaSegmentos[0][0] = 0;                    // Base = 0
    mv->tablaSegmentos[0][1] = tamano_codigo - 1;        // Tama�o del c�digo

    // Entrada 1: Segmento de datos
    mv->tablaSegmentos[1][0] = tamano_codigo;        // Base = fin del c�digo
    mv->tablaSegmentos[1][1] = 16384 - tamano_codigo; // Tama�o del datos

    // Las dem�s entradas (2-7) se inicializan a 0
    for (int i = 2; i < 8; i++) {
        for ( int j = 0; i < 2; j++)
            mv->tablaSegmentos[i][j] = 0;
    }

    // 7. Inicializar registros
    for (int i = 0; i < 32; i++) {
        mv->registros[i] = 0;
    }

    // Inicializar registros especiales
    mv->registros[26] = 0x00000000;  // CS: segmento de c�digo (tabla entrada 0)
    mv->registros[27] = 0x00010000;  // DS: segmento de datos (tabla entrada 1)
    mv->registros[3] = 0x00000000;   // IP: comienza en inicio del c�digo
    mv->registros[17] = 0;                      // Condition Code inicial

    printf("Programa cargado: %d bytes de codigo\n", tamano_codigo);
}
/*
void muestraCS(maquinaVirtual mv) {
    printf("=== SEGMENTO DE C�DIGO (CS) ===\n");

    // Obtener informaci�n del segmento de c�digo desde tabla de segmentos
    int base_cs = mv.tablaSegmentos[0][0];      // Base del segmento de c�digo
    int tamano_cs = mv.tablaSegmentos[0][1];    // Tama�o del segmento de c�digo

    printf("Base: 0x%04X, Tamano: %d bytes\n", base_cs, tamano_cs);
    printf("Direcci�n IP: 0x%04X\n", mv.registros[3]); // IP
    printf("\n");

    // Mostrar contenido en formato hexadecimal y ASCII
    printf("Direc.   Hexadecimal                         ASCII\n");
    printf("-------  ----------------------------------  --------\n");

    for (int i = 0; i < tamano_cs; i += 16) {
        // Direcci�n actual
        printf("0x%04X:  ", base_cs + i);

        // Bytes en hexadecimal
        for (int j = 0; j < 16; j++) {
            if (i + j < tamano_cs) {
                printf("%02X ", (unsigned char)mv.memoria[base_cs + i + j]);
            } else {
                printf("   "); // Espacios para alinear
            }

            if (j == 7) printf(" "); // Separador a mitad de l�nea
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

    // Mostrar tambi�n el valor del registro CS
    printf("\nRegistro CS: 0x%08X\n", mv.registros[26]);
}
*/

void ejecutarMV(maquinaVirtual *mv) {

    operando op[2];
    //cargar el vector de funciones
    Toperaciones v[32];
    cargaVF(v);
    //cargar el vector de funciones del sistema
    funcionSys vecLlamadas[2];
    loadSYSOperationArray(vecLlamadas);
    

    //Ciclo de ejecucion
    while( mv->registros[OPC] != 0x0F || mv->registros[IP] < 16384 ) {

        //leer instruccion apuntada por el registro IP
        leerInstruccion( *mv, op );
        //Almacenar el codigo de operacion en el registro OPC
        setReg(mv, OPC, getMem(mv, op[0]));
        //Guardar en los registros OP1 y OP2 los operandos de la instruccion
        setReg(mv, OP1, getMem(mv, op[1]));
        setReg(mv, OP2, getMem(mv, op[2]));
        //Ubicar en el registro IP la proxima instruccion a ejecutar
        setReg(mv, IP, mv->registros[IP] + 1 + (op[0].tipo != 0) + (op[1].tipo != 0) + (op[2].tipo != 0));
        //Realizar la operacion indicada por el codigo de operacion
        ejecutarOperacion(mv, mv->registros[OPC], op);
        //Repetir el proceso hasta encontrar la instruccion STOP}
    }
    printf("\nEjecucion finalizada\n");
}

void disassembler( maquinaVirtual mv ) {
    
}
