#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disassembler.h"
#include "../include/mv.h"


int verificarIntegridadVMX(FILE* arch);

int main(int argc, char *argv[]) {

    maquinaVirtual mv;

    char* vmx_file;
    char* vmi_file;
    char *ext;

    int desensamblador = 0;
    int parametros = 0;

    int tiene_vmx = 0;
    int tiene_vmi = 0;

    int version = 0;

    mv.memSize = MAX_MEM; // valor por defecto
    
    if (argc < 1) {
        fprintf(stderr, "Archivos insuficientes\n");
        return 0;
    }
{}
    // Verificar argumentos
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-d") == 0) {
            desensamblador = 1; 
        }

        if (strcmp(argv[i], "-p") == 0 && i == argc - 1) {
            parametros = 1; 
            
        }

        if (strncmp(argv[i], "-m", 2) == 0) {
            mv.memSize = atoi(&argv[i][2]);
        }

        ext = strrchr(argv[i], '.');
        if (ext != NULL) {

            if (strcmp(ext, ".vmx") == 0) {

                vmx_file = argv[i];
                tiene_vmx = 1;

            } else if (strcmp(ext, ".vmi") == 0) {

                vmi_file = argv[i];
                tiene_vmi = 1;

            }
        }
    }

    // Verificación mínima requerida
    if (!tiene_vmx && !tiene_vmi) {
        fprintf(stderr, "Error: Se requiere al menos un archivo .vmx o .vmi\n");

        return 1;
    }

    mv.memoria = (unsigned char *)malloc(mv.memSize * sizeof(unsigned char));
    FILE* archivo = fopen(vmx_file, "rb");

    if (!verificarIntegridadVMX(archivo)) {
        fclose(archivo);
        return 1;
    }

    printf("\nHello maquina virtual\n");

    leerMV(&mv, archivo, &version); // carga el archivo vmx en la memoria de la MV
    //printf("Version del archivo: %d\n", version);
    fclose(archivo);

    if (desensamblador)
    {
        printf("\n=== DESASSEMBLER ===\n");
        disassembler(mv);
    }

    ejecutarMV(&mv, version);     // solo ejecuta la MV

    printf("\nFIN MAQUINA VIRTUAL\n");


    return 0;
}


int verificarIntegridadVMX(FILE* arch) {
    // Verificar que el archivo tiene al menos el tama o m nimo de cabecera
    fseek(arch, 0, SEEK_END);
    long file_size = ftell(arch);
    fseek(arch, 0, SEEK_SET);

    if (file_size < 7) { // 5 (VMX25) + 1 (versi n) + 2 (tama o)
        fprintf(stderr, "Error: Archivo demasiado peque o\n");
        return 0;
    }

    return 1;
}
