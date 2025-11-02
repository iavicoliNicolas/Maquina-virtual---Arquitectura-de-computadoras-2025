#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disassembler.h"
#include "../include/mv.h"
#include "../include/paramsegment.h"
#include "../include/vmi.h"
#include <sys/stat.h>
int verificarIntegridadVMX(FILE* arch);

int main(int argc, char *argv[]) {

    maquinaVirtual mv;

    char* vmx_file;
    char* vmi_file;
    char *ext;

    int desensamblador = 0;
    int parametros = 0;
    int indiceParametros = -1;   //  indice donde aparece "-p" //agrega Euge

    int tiene_vmx = 0;
    int tiene_vmi = 0;

    int version = 0;
    int paramSize = 0;  //agrega Euge
    mv.memSize = MAX_MEM; // valor por defecto

    // ================================================================
    //   SECCIÓN 1: LECTURA Y ANÁLISIS DE LOS ARGUMENTOS DE CONSOLA
    // ================================================================
    if (argc < 1) {
        fprintf(stderr, "Archivos insuficientes\n");
        return 0;
    }

    // Verificar argumentos
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-d") == 0) {
            desensamblador = 1;
        }

        if (strcmp(argv[i], "-p") == 0) {
            parametros = 1;
            indiceParametros = i + 1;  // guardo posicion del primer parametro, para saber que a partir de aca solo hay parametros que se guardan en el param segment
          break;                     //  el resto son parametros, corto aca

        }
        if (strncmp(argv[i], "-m", 2) == 0) {
            mv.memSize = atoi(&argv[i][2]);
        }

        ext = strrchr(argv[i], '.');
        if (ext != NULL) {
            if (strcmp(ext, ".vmx") == 0) {

                vmx_file = argv[i];
                tiene_vmx = 1;

            } else
               if (strcmp(ext, ".vmi") == 0) {
                  vmi_file = argv[i];
                  tiene_vmi = 1;
               }
        }
    }
    // ================================================================
    //   SECCIÓN 2: CASOS POSIBLES SEGÚN LOS ARCHIVOS RECIBIDOS
    // ================================================================

    // Verificación mínima requerida
    if (!tiene_vmx && !tiene_vmi) {
        fprintf(stderr, "Error: Se requiere al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    mv.memoria = (unsigned char *)malloc(mv.memSize * sizeof(unsigned char));

    // Si el usuario pasa SOLO un archivo .vmi, se trata de reanudar una ejecución guardada
    if (tiene_vmi && !tiene_vmx) {
        // Carga completa del estado de la máquina virtual desde el archivo .vmi
        if (!leerVMI(&mv, vmi_file)) {
            fprintf(stderr, "Error al leer el archivo VMI.\n");
            return 1;
        }
        // Continúa la ejecución desde donde se guardó (IP restaurado)
        ejecutarMV(&mv, 2);
        return 0;
    }
    // Si hay parámetros, crear el Param Segment antes de leer el VMX

    // ================================================================
    //   SECCIÓN 3: CREACIÓN DEL PARAM SEGMENT (si hay -p)
    // ================================================================
   if (parametros && indiceParametros > 0 && indiceParametros < argc)
    {
      int cantParams = argc - indiceParametros;
      printf("Creando Param Segment con %d parámetros...\n", cantParams);//-------> para verificar BORRAR
      paramSize = crearParamSegment(&mv, cantParams, &argv[indiceParametros]);

      //para verificar funcionamiento--------------------------------------------->borrar
      for (int i = 0; i < paramSize; i++) {
        printf("%02X ", mv.memoria[i]);
        if ((i+1) % 16 == 0)
           printf("\n");
      }
      //fin borrar
    printf("\nTamano Param Segment: %d bytes\n", paramSize);//para verificar funcionamiento---------------------------->borrar
    }
    // ================================================================
    //   SECCIÓN 4: LECTURA DEL ARCHIVO .VMX (PROGRAMA)
    // ================================================================
    FILE* archivo = fopen(vmx_file, "rb");

    if (!verificarIntegridadVMX(archivo)) {
        fclose(archivo);
        return 1;
    }

    printf("\nHello maquina virtual\n");


    leerMV(&mv, archivo, &version, paramSize);// carga el archivo vmx en la memoria de la MV //Cambio: agregue el paramsize. Al pasar paramSize, estas garantizando que cada segmento del VMX se carga en memoria sin superponerse con los parametros.
    //printf("Version del archivo: %d\n", version);
    fclose(archivo);

    // ================================================================
    //   SECCIÓN 5: DESENSAMBLADO (solo si se usó -d)
    // ================================================================
    if (desensamblador)
    {   printf("\n=== DESASSEMBLER ===\n");
        disassembler(mv,version);
    }
    // ================================================================
    //   SECCIÓN 6: EJECUCIÓN DEL PROGRAMA
    // ================================================================
    ejecutarMV(&mv, version);
    // Si se indicó un archivo .vmi además del .vmx,
    // se guarda una imagen del estado final (para continuar después)
    if (tiene_vmi)
        guardarVMI(&mv, vmi_file);

    printf("\nFIN MAQUINA VIRTUAL\n");


    return 0;
}


int verificarIntegridadVMX(FILE* arch) {
    // Verificar que el archivo tiene al menos el tamaño minimo de cabecera
    fseek(arch, 0, SEEK_END);
    long file_size = ftell(arch);
    fseek(arch, 0, SEEK_SET);

    if (file_size < 7) { // 5 (VMX25) + 1 (version) + 2 (tamaño)
        fprintf(stderr, "Error: Archivo demasiado pequeno\n");
        return 0;
    }

    return 1;
}

