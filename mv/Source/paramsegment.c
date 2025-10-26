#include <string.h>
#include "paramsegment.h"
#include "mv.h"

int crearParamSegment(maquinaVirtual *mv, int argc, char **argv) {
    if (argc <= 0 || argv == NULL)
        return 0;

    int offset = 0;
    int i;

    // Copiar las cadenas de caracteres
    for (i = 0; i < argc; i++) {
        int len = strlen(argv[i]) + 1; // +1 por el '\0'
        memcpy(&mv->memoria[offset], argv[i], len);
        offset += len;
    }

    // Crear los punteros al final del segmento
    int acum = 0;
    for (i = 0; i < argc; i++) {
        unsigned int ptr = 0x00000000 | acum; // 16 bits altos = 0x0000, bajos = offset del string
        memcpy(&mv->memoria[offset], &ptr, sizeof(unsigned int));
        offset += 4;
        acum += strlen(argv[i]) + 1;
    }

    // Actualizar tabla de segmentos y registro PS
    mv->tablaSegmentos[0][0] = 0;      // Base física del Param Segment
    mv->tablaSegmentos[0][1] = offset; // Tamano total del segmento
    mv->registros[PS] = 0x00000000;    // Registro PS apunta al inicio

    return offset; // Retorna el tamano total del Param Segment
}