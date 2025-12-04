#include <stdio.h>
#include <stdlib.h>
#include "mv.h"
#include "operando.h"
#include "funciones.h"
#include "disassembler.h"
#include "paramsegment.h"

int corrigeSize(int size)
{
    int aux1 = 0, aux2 = 0;
    aux1 = (size >> 8) & 0x00FF;
    aux2 = (size & 0x00FF) << 8;
    return aux2 | aux1;
}

// Convierte una dirección lógica a una dirección física
int logicoAFisico(maquinaVirtual *mv, int direccionLogica) {

    printf("Direccion logica: 0x%08X\n", direccionLogica);

    int segmento = (direccionLogica >> 16) & 0xFF;
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

//----------------------------Funciones principal de lectura de la MV-----------------------------//

void leerMV(maquinaVirtual *mv, FILE* arch, int *version, int paramSize) {
    char cabecera[6];
    unsigned short int tamano_codigo, entry_point = 0;
    unsigned short int longitudSeg[6]; // Ahora 6: Code, Data, Extra, Stack, Const, Entry

    // 1. Leer identificador
    fread(cabecera, sizeof(char), 5, arch);
    cabecera[5] = '\0';

    if (strcmp(cabecera, "VMX25") != 0) {
        fprintf(stderr, "Error: Formato de archivo invalido\n");
        exit(EXIT_FAILURE);
    }

    // 2. Leer versión
    fread(version, sizeof(unsigned char), 1, arch);

    // 3. Inicializar registros
    for (int i = 0; i < 32; i++) {
        mv->registros[i] = 0;
    }

    // 4. Inicializar tabla de segmentos (todo inválido)
    for (int i = 0; i < 8; i++) {
        mv->tablaSegmentos[i][0] = -1;
        mv->tablaSegmentos[i][1] = 0;
    }

    // 5. Manejar según versión
    if (*version == 1) {
        /*********************** VERSIÓN 1 ***********************/

        // Leer tamaño del código
        fread(&tamano_codigo, sizeof(unsigned short int), 1, arch);
        tamano_codigo = corrigeSize(tamano_codigo);

        // Verificar que cabe en memoria
        if (tamano_codigo > mv->memSize) {
            fprintf(stderr, "Error: Codigo demasiado grande (%d bytes)\n", tamano_codigo);
            exit(EXIT_FAILURE);
        }

        // Cargar código en memoria (segmento de código)
        fread(mv->memoria, sizeof(char), tamano_codigo, arch);

        // Configurar tabla de segmentos
        mv->tablaSegmentos[0][0] = 0;                     // Base = 0
        mv->tablaSegmentos[0][1] = tamano_codigo;         // Tamaño del código

        mv->tablaSegmentos[1][0] = tamano_codigo;         // Base = fin del código
        mv->tablaSegmentos[1][1] = mv->memSize - tamano_codigo; // Tamaño de datos

        // Configurar registros
        mv->registros[CS] = 0x00000000;  // Segmento 0, offset 0
        mv->registros[DS] = 0x00010000;  // Segmento 1, offset 0
        mv->registros[ES] = 0xFFFFFFFF;  // No existe
        mv->registros[SS] = 0xFFFFFFFF;  // No existe
        mv->registros[KS] = 0xFFFFFFFF;  // No existe
        mv->registros[PS] = 0xFFFFFFFF;  // No existe

        // Inicializar IP al inicio del Code Segment
        mv->registros[IP] = mv->registros[CS];

        printf("MV1 cargado: %d bytes de codigo\n", tamano_codigo);

    } else if (*version == 2) {
        /*********************** VERSIÓN 2 ***********************/

        // Leer todos los tamaños (6 valores: Code, Data, Extra, Stack, Const, Entry)
        fread(longitudSeg, sizeof(unsigned short int), 6, arch);
        for (int i = 0; i < 6; i++) {
            longitudSeg[i] = corrigeSize(longitudSeg[i]);
        }

        // Tamaños individuales
        unsigned short int code_size = longitudSeg[0];
        unsigned short int data_size = longitudSeg[1];
        unsigned short int extra_size = longitudSeg[2];
        unsigned short int stack_size = longitudSeg[3];
        unsigned short int const_size = longitudSeg[4];
        entry_point = longitudSeg[5];  // Offset dentro del Code Segment

        // Calcular offset inicial (Param Segment si existe)
        int offset_actual = (paramSize > 0) ? paramSize : 0;
        int idx_tabla = 0;  // Índice actual en tabla de segmentos

        // 5.1. Param Segment (si existe)
        if (paramSize > 0) {
            mv->tablaSegmentos[idx_tabla][0] = 0;
            mv->tablaSegmentos[idx_tabla][1] = paramSize;
            mv->registros[PS] = (idx_tabla << 16);  // Índice en bits altos
            idx_tabla++;
        } else {
            mv->registros[PS] = 0xFFFFFFFF;
        }

        // 5.2. Const Segment (si existe)
        if (const_size > 0) {
            mv->tablaSegmentos[idx_tabla][0] = offset_actual;
            mv->tablaSegmentos[idx_tabla][1] = const_size;
            mv->registros[KS] = (idx_tabla << 16);
            offset_actual += const_size;
            idx_tabla++;
        } else {
            mv->registros[KS] = 0xFFFFFFFF;
        }

        // 5.3. Code Segment (SIEMPRE existe)
        mv->tablaSegmentos[idx_tabla][0] = offset_actual;
        mv->tablaSegmentos[idx_tabla][1] = code_size;
        mv->registros[CS] = (idx_tabla << 16);
        offset_actual += code_size;
        idx_tabla++;

        // 5.4. Data Segment (si existe)
        if (data_size > 0) {
            mv->tablaSegmentos[idx_tabla][0] = offset_actual;
            mv->tablaSegmentos[idx_tabla][1] = data_size;
            mv->registros[DS] = (idx_tabla << 16);
            offset_actual += data_size;
            idx_tabla++;
        } else {
            mv->registros[DS] = 0xFFFFFFFF;
        }

        // 5.5. Extra Segment (si existe)
        if (extra_size > 0) {
            mv->tablaSegmentos[idx_tabla][0] = offset_actual;
            mv->tablaSegmentos[idx_tabla][1] = extra_size;
            mv->registros[ES] = (idx_tabla << 16);
            offset_actual += extra_size;
            idx_tabla++;
        } else {
            mv->registros[ES] = 0xFFFFFFFF;
        }

        // 5.6. Stack Segment (si existe)
        if (stack_size > 0) {
            mv->tablaSegmentos[idx_tabla][0] = offset_actual;
            mv->tablaSegmentos[idx_tabla][1] = stack_size;
            mv->registros[SS] = (idx_tabla << 16);
            offset_actual += stack_size;
            idx_tabla++;

            // Inicializar SP: tope de la pila (base + tamaño)
            mv->registros[SP] = mv->tablaSegmentos[idx_tabla-1][0] + stack_size;
        } else {
            mv->registros[SS] = 0xFFFFFFFF;
            mv->registros[SP] = 0xFFFFFFFF;
        }

        // Verificar que todo cabe en memoria
        if (offset_actual > mv->memSize) {
            fprintf(stderr, "Error: Memoria insuficiente. Necesario: %d, Disponible: %d\n",
                    offset_actual, mv->memSize);
            exit(EXIT_FAILURE);
        }

        // CARGAR CONTENIDOS DESDE ARCHIVO

        // 5.7. Cargar Code Segment
        fread(&mv->memoria[mv->tablaSegmentos[idx_tabla-1][0]],
              sizeof(char), code_size, arch);

        // 5.8. Cargar Const Segment (si existe)
        if (const_size > 0) {
            // Buscar índice del Const Segment en la tabla
            int idx_const = (paramSize > 0) ? 1 : 0;
            fread(&mv->memoria[mv->tablaSegmentos[idx_const][0]],
                  sizeof(char), const_size, arch);
        }

        // 5.9. Configurar IP con entry point
        mv->registros[IP] = mv->registros[CS] | entry_point;

        printf("MV2 cargado: Code=%d, Data=%d, Const=%d, Entry=0x%04X\n",
               code_size, data_size, const_size, entry_point);

    } else {
        fprintf(stderr, "Error: Versión no soportada: %d\n", *version);
        exit(EXIT_FAILURE);
    }

    // Inicializar Condition Code
    mv->registros[CC] = 0;

    // Inicializar BP (no requiere valor inicial específico)
    mv->registros[BP] = 0;
}

//-------------------------Funciones de la ejecucion de la MV-------------------------//

// Función para leer la instrucción en la posición del registro IP
void leerInstruccion(maquinaVirtual *mv, unsigned char *operacion, operando *operandos) {

    //consigo la direccion fisica de la instruccion
    int ip = logicoAFisico(mv, mv->registros[IP]);

    *operacion = (unsigned char)mv->memoria[ip] & 0x1F; // 5 bits menos significativos
    operandos[0].tipo = (mv->memoria[ip] >> 4) & 0x03; // bits 5 y 6
    operandos[1].tipo = (mv->memoria[ip] >> 6) & 0x03; // bits 7 y 8

}

// Función principal de ejecución de la máquina virtual
void ejecutarMV(maquinaVirtual *mv, int version) {

    unsigned char operacion = 0;
    operando operandos[2];
    //cargar el vector de funciones
    Toperaciones v[32];
    cargaVF(v);

    funcionSys vecLlamadas[0xF];
    loadSYSOperationArray(vecLlamadas);

    int posCS = (mv->registros[CS] >> 16) & 0xFFFF, condicion;

    //Ciclo de ejecucion
    printf("\n=== INICIO EJECUCION MAQUINA VIRTUAL ===\n");

    while( mv->registros[IP] < mv->tablaSegmentos[posCS][1] && mv->registros[IP] >= 0) //mientras IP < limite del segmento de codigo
    {
        printf("\n=============Ciclo================\n");
        printf("\n--- Ejecucion de instruccion en IP=0x%04X ---\n", mv->registros[IP]);
        //leer instruccion apuntada por el registro IP
        leerInstruccion( mv, &operacion, operandos);

        //Almacenar el codigo de operacion en el registro OPC
        setReg(mv, OPC, operacion );

        //Recuperar los operandos de la instruccion y almacenarlos en el arreglo operandos
        recuperaOperandos(mv, operandos, mv->registros[IP]);

        //Guardar en los registros OP1 y OP2 los operandos de la instruccion
        setRegOP(mv, OP1, operandos[0], operandos[0].tipo);
        setRegOP(mv, OP2, operandos[1], operandos[1].tipo);

        //Ubicar en el registro IP la proxima instruccion a ejecutar
        setReg(mv, IP, mv->registros[IP] + 1 + operandos[0].tipo + operandos[1].tipo);

        int op[2];
        op[0] = mv->registros[OP1];
        op[1] = mv->registros[OP2];

        switch ( version ) {
            case 1: condicion = ( 0x10 <= operacion && operacion <= 0x1F ) || (0x00 <= operacion && operacion <= 0x08) || ( operacion == 0x0F);
                break;
            case 2: condicion = ( 0x10 <= operacion && operacion <= 0x1F ) || (0x00 <= operacion && operacion <= 0x08) || ( operacion == 0x0F) || ( operacion == 0x0E ) || ( 0x0B <= operacion && operacion <= 0x0D);
                break;
        }

        if ( ! condicion ) {

            fprintf(stderr, "Error: Operacion invalida: %d\n", operacion);
            exit(EXIT_FAILURE);
        } else {

            //ejecutar la operacion
            v[operacion](mv, op);
        }
        //DEBUG: Mostrar estado mínimo

        //Mostrar instrucción (opcional, solo si -d flag)
        const char* mnemonic = mnemonicos[operacion];
        printf("%s ", mnemonic ? mnemonic : "???");
        if (operandos[0].tipo != 0) {
            imprimeOperando(operandos[0]);
            if (operandos[1].tipo != 0) {
                printf(", ");
                imprimeOperando(operandos[1]);
            }
        }
        printf("\n");

        printf("\nIP=0x%08X | ", mv->registros[IP]);

        // Mostrar LAR/MAR/MBR solo si tienen valores de acceso previo
        if (mv->registros[LAR] != 0) {
            printf("LAR=0x%08X ", mv->registros[LAR]);
        }
        if (mv->registros[MAR] != 0) {
            printf("MAR=0x%08X ", mv->registros[MAR]);
        }
        if (mv->registros[MBR] != 0) {
            printf("MBR=0x%08X ", mv->registros[MBR]);
        }
        printf("\n");
    }
    printf("\nEjecucion finalizada\n");
}

//------------------ Funcion de acceso empleo de archivo .vmi ------------------//

/*
Header .vmi:
- 5 bytes: "VMI25"
version
- 1 byte: version (actualmente 2)
tamaño de la memoria
-2 bytes: tamaño de la memoria (little-endian)

Registros de la MV
- 32 registros de 4 bytes cada uno (little-endian) = 128 bytes
Entradas de tabla descriptora de segmentos
- 8 entradas de 4 bytes cada una (little-endian) = 32 bytes
Contenido de la memoria
- n bytes: contenido de la memoria de la MV
*/

void escribeVMI(maquinaVirtual *mv, const char *nombreArchivo) {
    FILE *arch = fopen(nombreArchivo, "wb");
    if (arch == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo VMI: %s\n", nombreArchivo);
        exit(EXIT_FAILURE);
    }

    // Escribir cabecera "VMI25"
    const char *cabecera = "VMI25";
    fwrite(cabecera, sizeof(char), 5, arch);

    // Escribir versión (1 byte)
    unsigned char version = 2;
    fwrite(&version, sizeof(unsigned char), 1, arch);

    // Escribir tamaño de memoria (4 bytes, little-endian)
    unsigned int memSizeLE = ((mv->memSize & 0xFF00) >> 8) | ((mv->memSize & 0x00FF) << 8);
    fwrite(&memSizeLE, sizeof(unsigned int), 1, arch);

    // Escribir registros de la MV (32 registros de 4 bytes cada uno, little-endian)
    for (int i = 0; i < MAX_REG; i++) {
        unsigned int regLE = ((mv->registros[i] & 0xFF00FF00) >> 8) | ((mv->registros[i] & 0x00FF00FF) << 8);
        fwrite(&regLE, sizeof(unsigned int), 1, arch);
    }

    // Escribir entradas de la tabla descriptora de segmentos (8 entradas de 4 bytes cada una, little-endian)
    for (int i = 0; i < MAX_SEG; i++) {
        for (int j = 0; j < 2; j++) {
            unsigned int entradaLE = ((mv->tablaSegmentos[i][j] & 0xFF00FF00) >> 8) | ((mv->tablaSegmentos[i][j] & 0x00FF00FF) << 8);
            fwrite(&entradaLE, sizeof(unsigned int), 1, arch);
        }
    }

    // Escribir contenido de la memoria
    fwrite(mv->memoria, sizeof(unsigned char), mv->memoriaUsada, arch);

    fclose(arch);
}

void leeVMI(maquinaVirtual *mv, const char *nombreArchivo) {

    FILE *arch = fopen(nombreArchivo, "rb");

    if (arch == NULL) {
        fprintf(stderr, "Error: No se pudo abrir el archivo VMI: %s\n", nombreArchivo);
        exit(EXIT_FAILURE);
    }

    // Leer cabecera
    char cabecera[6];
    fread(cabecera, sizeof(char), 5, arch);
    cabecera[5] = '\0';
    if (strcmp(cabecera, "VMI25") != 0) {
        fprintf(stderr, "Error: Formato de archivo VMI no valido: %s\n", nombreArchivo);
        fclose(arch);
        exit(EXIT_FAILURE);
    }

    // Leer tamaño de memoria
    unsigned int memSizeLE;
    fread(&memSizeLE, sizeof(unsigned int), 1, arch);
    mv->memSize = ((memSizeLE & 0xFF00) >> 8) | ((memSizeLE & 0x00FF) << 8);

    // Leer registros de la MV
    for (int i = 0; i < MAX_REG; i++) {
        unsigned int regLE;
        fread(&regLE, sizeof(unsigned int), 1, arch);
        mv->registros[i] = ((regLE & 0xFF00FF00) >> 8) | ((regLE & 0x00FF00FF) << 8);
    }

    // Leer entradas de la tabla descriptora de segmentos
    for (int i = 0; i < MAX_SEG; i++) {
        for (int j = 0; j < 2; j++) {
            unsigned int entradaLE;
            fread(&entradaLE, sizeof(unsigned int), 1, arch);
            mv->tablaSegmentos[i][j] = ((entradaLE & 0xFF00FF00) >> 8) | ((entradaLE & 0x00FF00FF) << 8);
        }
    }

    // Leer contenido de la memoria
    fread(mv->memoria, sizeof(unsigned char), mv->memoriaUsada, arch);

    fclose(arch);
}
//--------------------------Fin uso de .vmi-----------------------------//


/* para refactorizar y encapsular mejor mas tarde
void leerMemoria(maquinaVirtual *mv, int direccion, unsigned char *valor) {
    int dirF = logicoAFisico(mv, direccion); //convertir direccion logica a fisica
    *valor = mv->memoria[dirF];
}

void escribirMemoria(maquinaVirtual *mv, int direccion, unsigned char valor) {
    int dirF = logicoAFisico(mv, direccion); //convertir direccion logica a fisica
    mv->memoria[dirF] = valor;
}
*/

