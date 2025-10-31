#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/vmi.h"

#define VMI_ID "VMI25"
#define VMI_VERSION 1

int guardarVMI(maquinaVirtual *mv, const char *nombreArchivo) {
    FILE *file = fopen(nombreArchivo, "wb");
    if (!file){
         fprintf(stderr, "Error: no se pudo crear el archivo %s\n", nombreArchivo);
        return 0;
    }
    unsigned short memKiB = (unsigned short)(mv->memSize / 1024);
    fwrite(VMI_ID, sizeof(char), 5, file);
    fputc(VMI_VERSION, file);
    fwrite(&memKiB, sizeof(unsigned short), 1, file);

    fwrite(mv->registros, sizeof(int), 32, file);
    fwrite(mv->tablaSegmentos, sizeof(int), 8 * 2, file);
    fwrite(mv->memoria, sizeof(unsigned char), mv->memSize, file);

    fclose(file);
    //imagen VMI guardada
    return 1;
}

int leerVMI(maquinaVirtual *mv, const char *nombreArchivo) {
    FILE *file = fopen(nombreArchivo, "rb");
     if (!file) {
        fprintf(stderr, "Error: no se pudo abrir el archivo %s\n", nombreArchivo);
        return 0;
    }

    char id[6];
    fread(id, sizeof(char), 5, file);
    id[5] = '\0';
    if (strcmp(id, VMI_ID) != 0) {
        fclose(file);
        return 0;
    }

    unsigned char version;
    fread(&version, sizeof(unsigned char), 1, file);
    if (version != VMI_VERSION) {
        fprintf(stderr, "Error: versión no soportada (%d)\n", version);
        fclose(file);
        return 0;
    }

    unsigned short memKiB;
    fread(&memKiB, sizeof(unsigned short), 1, file);
    mv->memSize = memKiB * 1024;
    mv->memoria = (unsigned char *)malloc(mv->memSize);
    if (!mv->memoria) {
        fclose(file);
        return 0;
    }

    fread(mv->registros, sizeof(int), 32, file);
    fread(mv->tablaSegmentos, sizeof(int), 8 * 2, file);
    fread(mv->memoria, sizeof(unsigned char), mv->memSize, file);

    fclose(file);
    //imagen VMI cargada correctamente
    return 1;
}
