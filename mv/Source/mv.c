#include <stdio.h>
#include <stdlib.h>
#include "mv.h"
#include "operando.h"
#include "funciones.h"
#include "disassembler.h"

int corrigeSize(int size)
{
    int aux1 = 0, aux2 = 0;
    aux1 = (size >> 8) & 0x00FF;
    aux2 = (size & 0x00FF) << 8;
    return aux2 | aux1;
}

// Convierte una dirección lógica a una dirección física
int logicoAFisico(maquinaVirtual *mv, int direccionLogica) {

    int segmento = (direccionLogica >> 16) & 0xFFFF;
    int desplazamiento = direccionLogica & 0xFFFF;

    // Verificar que el segmento es válido
    if (segmento < 0 || segmento >= 8) {
        fprintf(stderr, "Error: Segmento inválido: %d\n", segmento);
        exit(EXIT_FAILURE);
    }

    // Calcular la dirección física
    int base = mv->tablaSegmentos[segmento][0];
    int limite = mv->tablaSegmentos[segmento][1];

    if (desplazamiento < 0 || desplazamiento >= limite) {
        fprintf(stderr, "Error: Desplazamiento fuera de rango: %d\n", desplazamiento);
        exit(EXIT_FAILURE);
    }

    return base + desplazamiento;
}

void setReg(maquinaVirtual *mv, int reg, int valor) {
    mv->registros[reg] = valor;
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
    for(int j=0;j<tamano_codigo;j++)
    {
        printf("%02X ", (unsigned char)mv->memoria[j]);
    }
    printf("\n");
    

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

    mv->registros[CS] = 0x00000000;  // CS: segmento de c digo (tabla entrada 0)
    mv->registros[DS] = 0x00010000;  // DS: segmento de datos (tabla entrada 1)
    mv->registros[IP] = 0x00000000;   // IP: comienza en inicio del c digo
    mv->registros[CC] = 0;                      // Condition Code inicial

    printf("Programa cargado: %d bytes de codigo\n", tamano_codigo);
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

void leerPrimerByte(maquinaVirtual *mv, char *operacion, char *tipoA, char *tipoB, int ip) {
    printf("Valor de ip: %x\n", ip); //DEBUG
    printf("valor que hay en la memoria en la posicion de ip: %x\n", mv->memoria[ip]); //DEBUG
    *operacion = mv->memoria[ip] & 0x1F; // 5 bits menos significativos
    *tipoA = (mv->memoria[ip] >> 4) & 0x03; // bits 5 y 6
    *tipoB = (mv->memoria[ip] >> 6) & 0x03; // bits 7 y 8
}

void leerInstruccion(maquinaVirtual *mv, char *operacion, operando *op) {

    //consigo la direccion fisica de la instruccion
    int ip = logicoAFisico(mv, mv->registros[IP]); 
    //leo el primer byte de la instruccion
    leerPrimerByte(mv, operacion, &op[0].tipo, &op[1].tipo, ip);

}

void ejecutarMV(maquinaVirtual *mv) {
    char operacion;
    operando operandos[2];
    //cargar el vector de funciones
    Toperaciones v[32];
    cargaVF(v);

    funcionSys vecLlamadas[2];
    loadSYSOperationArray(vecLlamadas);

    //Ciclo de ejecucion
    while( mv->registros[IP] < mv->tablaSegmentos[0][1] && mv->registros[IP] >= 0) //mientras IP < limite del segmento de codigo
    { 
        printf("\n=============Ciclo================\n");
        printf("\n--- Ejecucion de instruccion en IP=0x%04X ---\n", mv->registros[IP]);
        //leer instruccion apuntada por el registro IP
        leerInstruccion( mv, &operacion, operandos);
        //Almacenar el codigo de operacion en el registro OPC
        setReg(mv, OPC, operacion );
        printf("Valor de opc: %x\n", mv->registros[OPC]);

        
        recuperaOperandos(mv, operandos, mv->registros[IP]);
        //Guardar en los registros OP1 y OP2 los operandos de la instruccion
        setRegOP(mv, OP1, operandos[0], operandos[0].tipo);
        setRegOP(mv, OP2, operandos[1], operandos[1].tipo);
        //Ubicar en el registro IP la proxima instruccion a ejecutar
        setReg(mv, IP, mv->registros[IP] + 1 + (operandos[0].tipo != 0) + (operandos[1].tipo != 0));
        //Realizar la operacion indicada por el codigo de operacion

        if (operacion < 0 || operacion >= 32) {
            fprintf(stderr, "Error: Operacion invalida: %d\n", operacion);
            exit(EXIT_FAILURE);
        }

        int op[2];
        op[0] = mv->registros[OP1];
        op[1] = mv->registros[OP2];

        setLAR(mv, mv->registros[DS], mv->registros[DS] & 0xFFFF); //actualizar LAR con el valor de DS
        //cargar MAR y MBR si es necesario
        setMAR(mv, 2, logicoAFisico(mv, mv->registros[MAR] & 0xFFFF));
        setMBR(mv, getMem(mv, mv->registros[MAR]));
        
        //ejecutar la operacion
        v[operacion](mv, op);
    }
    printf("\nEjecución finalizada\n");
}


