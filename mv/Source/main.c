#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mv.h"

int verificarIntegridadVMX(FILE* arch);

int main(int argc, char *argv[]) {
    int modo_disassembler=0;
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
    
    if (argc >= 3 && strcmp(argv[2], "-d") == 0) 
    {
        printf("\n=== DESASSEMBLER ===\n");
        disassembler(&mv);   
    }
    
    ejecutarMV(&mv);     // solo ejecuta la MV

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
