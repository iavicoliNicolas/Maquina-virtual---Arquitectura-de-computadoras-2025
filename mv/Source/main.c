#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disassembler.h"
#include "../include/mv.h"
#include "../include/paramsegment.h"

int verificarIntegridadVMX(FILE* arch);

int main(int argc, char *argv[]) {
    maquinaVirtual mv;

    // Inicializar variables
    char* vmx_file = NULL;
    char* vmi_file = NULL;
    char *ext;

    int desensamblador = 0;
    int parametros = 0;
    int indiceParametros = -1;

    int tiene_vmx = 0;
    int tiene_vmi = 0;
    int version = 0;
    int paramSize = 0;
    mv.memSize = MAX_MEM; // valor por defecto

    if (argc < 2) {
        fprintf(stderr, "Uso: %s [archivo.vmx] [archivo.vmi] [-d] [-mM] [-p param1 ...]\n", argv[0]);
        fprintf(stderr, "Se requiere al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    // 1. Buscar -p primero para validar posición
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            indiceParametros = i;
            break;
        }
    }

    // Validar que -p esté al final y tenga parámetros
    if (indiceParametros != -1) {
        if (indiceParametros >= argc - 1) {
            fprintf(stderr, "Error: -p requiere al menos un parámetro\n");
            return 1;
        }
        parametros = 1;
    }

    // 2. Procesar otros parámetros (solo antes de -p si existe)
    int limite = (indiceParametros != -1) ? indiceParametros : argc;
    for (int i = 1; i < limite; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            desensamblador = 1;
        }
        else if (strncmp(argv[i], "-m", 2) == 0) {
            mv.memSize = atoi(&argv[i][2]) * 1024; // Convertir KiB a bytes
            if (mv.memSize < 1024 || mv.memSize > 65536) {
                fprintf(stderr, "Error: Tamaño de memoria inválido. Usar entre 1 y 64 KiB\n");
                return 1;
            }
        }
        else if ((ext = strrchr(argv[i], '.')) != NULL) {
            if (strcmp(ext, ".vmx") == 0) {
                vmx_file = argv[i];
                tiene_vmx = 1;
            } else if (strcmp(ext, ".vmi") == 0) {
                vmi_file = argv[i];
                tiene_vmi = 1;
            }
        }
    }

    // 3. Validación de archivos
    if (!tiene_vmx && !tiene_vmi) {
        fprintf(stderr, "Error: Se requiere al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    // 4. Inicializar memoria
    mv.memoria = (unsigned char *)malloc(mv.memSize * sizeof(unsigned char));
    if (!mv.memoria) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        return 1;
    }

    // 5. Crear Param Segment si hay parámetros
    if (parametros && indiceParametros < argc - 1) {
        int cantParams = argc - indiceParametros - 1;
        printf("Creando Param Segment con %d parámetros...\n", cantParams);
        paramSize = crearParamSegment(&mv, cantParams, &argv[indiceParametros + 1]);
        printf("Tamaño Param Segment: %d bytes\n", paramSize);
    }

    // 6. Cargar archivo según tipo
    if (tiene_vmx) {
        FILE* archivo = fopen(vmx_file, "rb");
        if (!archivo) {
            fprintf(stderr, "Error: No se pudo abrir %s\n", vmx_file);
            free(mv.memoria);
            return 1;
        }

        if (!verificarIntegridadVMX(archivo)) {
            fclose(archivo);
            free(mv.memoria);
            return 1;
        }

        leerMV(&mv, archivo, &version, paramSize);
        fclose(archivo);

        printf("Archivo VMX cargado: %s (versión %d)\n", vmx_file, version);
    }

    if (tiene_vmi && !tiene_vmx) {
        // Solo .vmi: cargar imagen y continuar ejecución
        printf("Cargando imagen VMI: %s\n", vmi_file);
        if (!leeVMI(&mv, vmi_file)) {
            fprintf(stderr, "Error al cargar imagen VMI\n");
            free(mv.memoria);
            return 1;
        }
    }

    // 7. Disassembler si se solicita
    if (desensamblador && tiene_vmx) {
        printf("\n=== DISASSEMBLER ===\n");
        disassembler(mv);
    }

    // 8. Ejecutar
    if (tiene_vmx || tiene_vmi) {
        printf("\n=== EJECUTANDO ===\n");
        ejecutarMV(&mv, version);
    }

    // 9. Liberar recursos
    free(mv.memoria);
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

