#include "operando.h"

//obtiene el valor del operando de registro
int getReg(maquinaVirtual *mv, operando op);

//obtiene el valor del operando de memoria
int getMem(maquinaVirtual *mv, operando op);

//obtiene el valor del operando segun su tipo
int getOp(maquinaVirtual *mv, operando op) {

    int valor = 0;
    switch (op.tipo) {
        case 0: // Registro
            valor = getReg(mv, op);
            break;
        case 1: // Memoria
            valor = getMem(mv, op);
            break;
        case 2: // Inmediato
            valor = op.desplazamiento;
            break;
        default:
            fprintf(stderr, "Error: Tipo de operando desconocido %d\n", op.tipo);
            exit(EXIT_FAILURE);
    }
    return valor;
}

void setOp(maquinaVirtual *mv, operando op, int num) {

    switch (op.tipo) {
        case 0: // Registro
            setReg(mv, op.registro, num);
            break;
        case 1: // Memoria
            // Asumimos que op.desplazamiento es la direccion fisica ya calculada
            if (op.desplazamiento < 0 || op.desplazamiento >= 16384) {
                fprintf(stderr, "Error: Direccion de memoria fuera de rango: %d\n", op.desplazamiento);
                exit(EXIT_FAILURE);
            }
            mv->memoria[op.desplazamiento] = (char)(num & 0xFF); // Guardar solo el byte menos significativo
            break;
        case 2: // Inmediato
            fprintf(stderr, "Error: No se puede asignar a un operando inmediato\n");
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Error: Tipo de operando desconocido %d\n", op.tipo);
            exit(EXIT_FAILURE);
    }
}
void recuperaOperandos(maquinaVirtual *mv,operando *o,int ip) {
    
    char aux;
    int auxInt;

    for (int i; i < 2; i++) {
        switch (o[i].tipo) {
            case 1: // Registro
                aux = mv->memoria[logicoAFisico(mv, ip + 1 + i)];
                o[i].registro = aux & 0x1F; // 5 bits menos significativos
                o[i].segmentoReg = (aux >> 5) & 0x07; // bits 6, 7 y 8
                o[i].desplazamiento = 0; // No aplica
                break;
            case 2: //inmediato
                // Leer 4 bytes y formar un entero (little-endian)
                auxInt = 0;
                for (int j = 0; j < 4; j++) {
                    auxInt |= (mv->memoria[logicoAFisico(mv, ip + 1 + i + j)] & 0xFF) << (j * 8);
                }
                o[i].desplazamiento = auxInt;
                o[i].registro = 0; // No aplica
                o[i].segmentoReg = 0; // No aplica
                break;
            case 3: // Memoria
                aux = mv->memoria[logicoAFisico(mv, ip + 1 + i)];
                o[i].registro = aux & 0x1F; // 5 bits menos significativos
                o[i].segmentoReg = (aux >> 5) & 0x07; // bits 6, 7 y 8
                // Leer 4 bytes y formar un entero (little-endian)
                auxInt = 0;
                for (int j = 0; j < 4; j++) {
                    auxInt |= (mv->memoria[logicoAFisico(mv, ip + 1 + i + j)] & 0xFF) << (j * 8);
                }
                o[i].desplazamiento = auxInt;
                break;
            default:
                
    }
}
