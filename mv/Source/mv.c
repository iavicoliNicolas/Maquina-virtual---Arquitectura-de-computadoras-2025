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

/*void leerMV(maquinaVirtual *mv, FILE* arch, int *version) {*/
// CAMBIO: agrego offsetInicial (para respetar el Param Segment si existe)
void leerMV(maquinaVirtual *mv, FILE* arch, int *version, int paramSize) {
    char cabecera[6];
    unsigned short int tamano_codigo, constantes, longitudSeg[5], offset,offsetInicial;
    int i, totalLongitud = 0;
    int ultSegmento = 0;
    int ordenSegmento[6] = {PS,KS,CS,DS,ES,SS};


    // 1. Leer cabecera del archivo VMX
    fread(cabecera, sizeof(char), 5, arch);
    cabecera[5] = '\0'; // Null-terminate para comparación

    // Verificar identificador "VMX25"
    if (strcmp(cabecera, "VMX25") != 0) {
        fprintf(stderr, "Error: Formato de archivo invalido\n");
        exit(EXIT_FAILURE);
    }

    // 2. Leer versión
    fread(version, sizeof(unsigned char), 1, arch);
    if ( (*version) != 1 && (*version) != 2 ) {
        fprintf(stderr, "Error: Versión no soportada: %d\n", *version);
        exit(EXIT_FAILURE);
    }
   printf("DEBUG: version leída = %d\n", *version); //---------->BORRAR
    // 3. Leer tamaño del segmento de código (2 bytes, little-endian)
    fread(&tamano_codigo, sizeof(unsigned short int), 1, arch);
    tamano_codigo = corrigeSize(tamano_codigo);


    // Inicializar registros
    for (i = 0; i < 32; i++) {
        mv->registros[i] = 0;
    }

    // bloque de versiones
    switch (*version)
    {
        case 1: // Código específico para la versión 1

            // 3. Leer tamaño del segmento de código (2 bytes, little-endian)
            fread(&tamano_codigo, sizeof(unsigned short int), 1, arch);
            tamano_codigo = corrigeSize(tamano_codigo);

            // 4. Verificar que el código cabe en memoria
            if (tamano_codigo > MAX_MEM) {
                fprintf(stderr, "Error: Codigo demasiado grande (%d bytes)\n", tamano_codigo);
                exit(EXIT_FAILURE);
            }

            // 5. Cargar código en memoria (segmento de código)
            fread(mv->memoria, sizeof(char), tamano_codigo, arch);

            // 6. Inicializar tabla de descriptores de segmentos
            mv->tablaSegmentos[0][0] = 0;                     // Base = 0
            mv->tablaSegmentos[0][1] = tamano_codigo;         // Tamaño del código

            mv->tablaSegmentos[1][0] = tamano_codigo;         // Base = fin del código
            mv->tablaSegmentos[1][1] = 16384 - tamano_codigo; // Tamaño de datos
            // Las demás entradas (2-7) se inicializan a 0

            ultSegmento = 1;
            // Inicializar registros especiales

            mv->registros[CS] = 0x00000000;  // CS: segmento de c digo (tabla entrada 0)
            mv->registros[DS] = 0x00010000;  // DS: segmento de datos (tabla entrada 1)

            printf("Programa cargado: %d bytes de codigo\n", tamano_codigo);
            break;

        case 2: // Código específico para la versión 2

            fread(longitudSeg, sizeof(unsigned short int), 5, arch);
            for ( i = 0; i < 5; i++ )
            {
                longitudSeg[i] = corrigeSize(longitudSeg[i]);
                if (longitudSeg[i] > 0)
                {
                    totalLongitud += longitudSeg[i];
                    ultSegmento = i;

                    /*comenta Euge:mv->tablaSegmentos[i][0] = (i == 0) ? 0 : mv->tablaSegmentos[i-1][0] + longitudSeg[i-1];*/
                    //agrega Euge
                    ////offsetInicial para definir la base del primer segmento
                    mv->tablaSegmentos[i][0] = (i == 0)
                        ? offsetInicial
                        : mv->tablaSegmentos[i-1][0] + longitudSeg[i-1];
                    //fin agrega Euge

                    mv->tablaSegmentos[i][1] = longitudSeg[i];

                    // Asignar el valor del registro correspondiente
                    mv->registros[ordenSegmento[i]] = mv->tablaSegmentos[i][0] << 16; // Desplazar a la parte alta
                    printf("Registro %d asignado a: 0x%08X\n", ordenSegmento[i], mv->registros[ordenSegmento[i]]);

                    if ( i == 0 ) {
                        tamano_codigo = longitudSeg[i];
                    } else if ( i == 4) {
                        constantes = longitudSeg[i];
                    }

                } else {
                    mv->registros[ordenSegmento[i+1]] = -1; // Si el segmento no existe, el registro apunta a 0
                }
            }
            /*Comenta Euge: if ( totalLongitud > mv->memSize ) {
                fprintf(stderr, "Error: Código demasiado grande, la maquina virtual requiere mas longitud de memoria (%d bytes)\n", totalLongitud);
                exit(EXIT_FAILURE);
            } else {
                // leo segmento de codigo del archivo
                fread(mv->memoria, sizeof(char), tamano_codigo ,arch);
                // leo segmento de constantes del archivo
                fread(&mv->memoria[tamano_codigo], sizeof(char), constantes ,arch);
            }  */
           //// Agrega Euge
            //  Control de memoria total considerando el Param Segment
            if (totalLongitud + offsetInicial > mv->memSize) {
                fprintf(stderr, "Error: Memoria insuficiente. Se requieren %d bytes.\n", totalLongitud + offsetInicial);
                exit(EXIT_FAILURE);
            } else {
                fread(&mv->memoria[mv->tablaSegmentos[0][0]], sizeof(char), tamano_codigo, arch);
                fread(&mv->memoria[mv->tablaSegmentos[4][0]], sizeof(char), constantes, arch);
            }

            ////Salida de prueba borra
            printf("Const Segment cargado en [%d..%d), tamaño %d bytes\n",
            mv->tablaSegmentos[4][0],
             mv->tablaSegmentos[4][0] + mv->tablaSegmentos[4][1],
             constantes);
            //fin "borrar"
           ////   Fin Agrega
            break;
        default:
            break;
        }

    for ( i = ultSegmento+1; i < 8; i++) {
        for (int j = 0; j < 2; j++)
            mv->tablaSegmentos[i][j] = -1;
    }

    mv->registros[IP] = mv->registros[CS]; // Inicializar IP al inicio del segmento de código
    mv->registros[CC] = 0;                  // Condition Code inicial
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

    funcionSys vecLlamadas[2];
    loadSYSOperationArray(vecLlamadas);

    //Ciclo de ejecucion
    printf("\n=== INICIO EJECUCION MAQUINA VIRTUAL ===\n");
    //muestraCS(*mv);
    while( mv->registros[IP] < mv->tablaSegmentos[0][1] && mv->registros[IP] >= 0) //mientras IP < limite del segmento de codigo
    {
        printf("\n=============Ciclo================\n");
        printf("\n--- Ejecucion de instruccion en IP=0x%04X ---\n", mv->registros[IP]);
        //leer instruccion apuntada por el registro IP
        leerInstruccion( mv, &operacion, operandos);

        //Almacenar el codigo de operacion en el registro OPC
        setReg(mv, OPC, operacion );

        //Recuperar los operandos de la instruccion y almacenarlos en el arreglo operandos
        recuperaOperandos(mv, operandos, mv->registros[IP]);
        printf("Operacion: %d ", operacion);
        printf("Operando 1: Tipo=%d, Registro=%d, Desplazamiento=%d ", operandos[0].tipo, operandos[0].registro, operandos[0].desplazamiento);
        printf("Operando 2: Tipo=%d, Registro=%d, Desplazamiento=%d\n", operandos[1].tipo, operandos[1].registro, operandos[1].desplazamiento);

        //Guardar en los registros OP1 y OP2 los operandos de la instruccion
        setRegOP(mv, OP1, operandos[0], operandos[0].tipo);
        setRegOP(mv, OP2, operandos[1], operandos[1].tipo);
        //printf("op1 y op2 cargados en registros OP1 y OP2\n");
        //printf("OP1: 0x%08X, OP2: 0x%08X\n", mv->registros[OP1], mv->registros[OP2]);

        //Ubicar en el registro IP la proxima instruccion a ejecutar
        setReg(mv, IP, mv->registros[IP] + 1 + operandos[0].tipo + operandos[1].tipo);

        int op[2];
        op[0] = mv->registros[OP1];
        op[1] = mv->registros[OP2];

        if (operacion < 0 || operacion >= 32) {
            fprintf(stderr, "Error: Operacion invalida: %d\n", operacion);
            exit(EXIT_FAILURE);
        } else {
            //ejecutar la operacion
            //printf("Ejecutando operacion %d\n", (int)operacion);
            v[operacion](mv, op);
        }
        printf("Registros despues de la operacion:\n");
        printf("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", mv->registros[EAX], mv->registros[EBX], mv->registros[ECX], mv->registros[EDX]);
        scanf("%*c"); //espera a que el usuario presione enter para continuar
    }
    printf("\nEjecucion finalizada\n");
}


