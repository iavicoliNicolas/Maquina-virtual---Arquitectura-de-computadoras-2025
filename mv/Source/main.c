#include <stdio.h>
#include <stdlib.h>
#include "mv.h"

int verificarIntegridadVMX(FILE* arch);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Uso: %s archivo.vmx [-d]\n", argv[0]);
        return 1;
    }

    FILE* archivo = fopen(argv[1], "rb");
    if (!archivo) {
        perror("Error al abrir archivo");
        return 1;
    }

    if (!verificarIntegridadVMX(archivo)) {
        fclose(archivo);
        return 1;
    }

    printf("\nHello maquina virtual\n");

    maquinaVirtual mv;
    leerMV(&mv, archivo);
    fclose(archivo);

    // Continuar con la ejecución...

    printf("\nFIN MAQUINA VIRTUAL");

    return 0;
}

int verificarIntegridadVMX(FILE* arch) {
    // Verificar que el archivo tiene al menos el tamaño mínimo de cabecera
    fseek(arch, 0, SEEK_END);
    long file_size = ftell(arch);
    fseek(arch, 0, SEEK_SET);

    if (file_size < 7) { // 5 (VMX25) + 1 (versión) + 2 (tamaño)
        fprintf(stderr, "Error: Archivo demasiado pequeño\n");
        return 0;
    }

    return 1;
}
