#include <stdio.h>
#include "mv.h"
#include "operando.h"
#include "disassembler.h" // para poder escribir los nombres de los reg

//obtiene el valor del operando de registro
int getReg(maquinaVirtual *mv, int op) {
    //extraer registro de op
    int reg = op & 0xFF;
    //comprobar que el registro es valido
    if (reg < 0 || reg >= MAX_REG) {
        fprintf(stderr, "Error: Registro invalido: %d\n", reg);
        exit(EXIT_FAILURE);
    }
    return reg;
}

//obtiene el valor de un operando inmediato
int getInm(maquinaVirtual *mv, int op) {
    //extraer desplazamiento de op
    int desplazamiento = 0;
    desplazamiento = op & 0xFFFF;
    //signo extendido si es negativo
    if (desplazamiento & 0x8000) {
        desplazamiento |= 0xFFFF0000;
    }
    return desplazamiento;
}
//obtiene el valor del operando de memoria
int getMem(maquinaVirtual *mv, int op) {

    int valor = 0;

    //extraer registro y desplazamiento de op
    int reg = (op >> 16) & 0xFF;
    int desplazamiento = op & 0x0000FFFF;
 
    //calcular la direccion efectiva
    int dirL = mv->registros[reg] + desplazamiento;
    int dirF = logicoAFisico(mv, dirL); //convertir direccion logica a fisica
    printf("Direccion logica: 0x%08X, Direccion fisica: 0x%08X\n", dirL, dirF);

    //leer el valor de memoria (2 bytes)
    if (dirF < 0 || dirF + 1 >= MAX_MEM) {
        fprintf(stderr, "Error: Lectura de memoria fuera de limites: %d\n", dirF);
        exit(EXIT_FAILURE);
    } else {
        
        valor = ((mv->memoria[dirF] & 0x00FF) << 8) | (mv->memoria[dirF + 1] & 0x00FF);

        setLAR(mv,dirL); 
        setMAR(mv, 4, dirF);
        setMBR(mv, valor);
    }

    return valor;
}

//obtiene el valor del operando segun su tipo
int getOp(maquinaVirtual *mv, int op) {
    //extraer tipo del byte mas significativo de op
    int tipo = (op >> 24) & 0xFF;

    //segun el tipo llama a a la funcion correspondiente
    switch (tipo) {
        case 0: //no usado
            return 0;
        case 1: //registro
            return mv->registros[getReg(mv, op)];
        case 2: //inmediato
            return getInm(mv, op);
        case 3: //memoria
            return getMem(mv, op); // [ax + 3]
        default:
            fprintf(stderr, "Error: Tipo de operando invalido: %d\n", tipo);
            exit(EXIT_FAILURE);
    }
    return -1; //para evitar warning
}
//establece el valor del operando segun su tipo
void setOp(maquinaVirtual *mv, int op, int num) { //OP1 | OP2

    //extraer tipo del byte mas significativo de op
    int tipo = (op >> 24) & 0xFF;

    //segun el tipo llama a a la funcion correspondiente
    switch (tipo) {
        case 0: //no usado
            break;
        case 1: { //registro
            mv->registros[getReg(mv, op)] = num; 
            break;
        }
        case 2: //inmediato
            fprintf(stderr, "Error: No se puede escribir en un operando inmediato\n");
            exit(EXIT_FAILURE);
            break;
        case 3: { //memoria
            mv->memoria[getMem(mv, op)] = num; 
            break;
        }
        default:
            fprintf(stderr, "Error: Tipo de operando invalido: %d\n", tipo);
            exit(EXIT_FAILURE);
    }
}

void recuperaOperandos(maquinaVirtual *mv, operando *operandos, int ip) {
    
    char aux;
    int auxInt;

    for (int i = 1; i >=0; i--)
    {
        if (operandos[i].tipo != 0) { 
            ip++;
            auxInt = 0;
            operandos[i].registro = -1;
            operandos[i].desplazamiento = -1;
            if (operandos[i].tipo == 1) { //si es de registro

                aux = mv->memoria[ip];
                operandos[i].registro = aux;
                operandos[i].desplazamiento = -1;
    
            } else if (operandos[i].tipo == 2) { //si es inmediato

                auxInt |= mv->memoria[ip++];
                auxInt |= mv->memoria[ip] & 0x00FF;
                operandos[i].desplazamiento = auxInt;
    
            } else if (operandos[i].tipo == 3) { //si es de memoria
    
                aux = mv->memoria[ip];
                ip++;
                auxInt |= mv->memoria[ip++];
                auxInt |= mv->memoria[ip] & 0x00FF;
                operandos[i].desplazamiento = auxInt;
                operandos[i].registro = aux;
            }
        } 
    }
    if(operandos[0].tipo == 0){
        operandos[0].tipo = operandos[1].tipo;
        operandos[0].registro = operandos[1].registro;
        operandos[0].desplazamiento = operandos[1].desplazamiento;
        operandos[1].tipo = 0;
        operandos[1].registro = -1;
        operandos[1].desplazamiento = -1;
    }   
}

void imprimeOperando(operando op) {
    switch (op.tipo) {
        case 0: // ninguno
            break;

        case 1: // registro
            if (op.registro >= 0 && op.registro < 32 && nombres_registros[op.registro])
                printf("%s", nombres_registros[op.registro]);
            else
                printf("reg%d", op.registro); // por si es reservado
            break;

        case 2: // inmediato 
            printf("%d", op.desplazamiento);
            break;

        case 3: // memoria 
            printf("[");
            if (op.registro != 0 && op.registro < 32 && nombres_registros[op.registro]) {
                printf("%s", nombres_registros[op.registro]);
                if (op.desplazamiento) {
                    printf(" + %d", op.desplazamiento);
                }
            } else {
                // acceso directo a memoria (sin registro base)
                printf("%d", op.desplazamiento);
            }
            printf("]");
            break;
    }
}

