#include <stdio.h>
#include "mv.h"
#include "operando.h"
#include "disassembler.h" // para poder escribir los nombres de los reg



void imprimeOperando(operando op) {
    printf("02222");
    switch (op.tipo) {
        case 0: // ninguno
            break;
        case 1: // registro
            if (op.registro >= 0 && op.registro < 32 && nombres_registros[op.registro])
                printf("%s", nombres_registros[op.registro]);
            else
                printf("registro reservado"); //eliminar al saber que funciona
            break;
        case 2: // inmediato (valor)
            printf("%d", op.desplazamiento);
            break;
        case 3: // memoria 
        {   printf("[ ");
            if(op.registro>=0 && op.registro<32 && nombres_registros[op.registro])
            {  printf("%s ",nombres_registros[op.registro]);
               if(op.desplazamiento)
                 printf(" + ");
            }
            if(op.desplazamiento)
               printf("%d ]",op.desplazamiento);
            else
               printf("]");                
            break;
        }  
    } 
        
        
    
}

int getRegOp(maquinaVirtual *mv, operando op) { return 0; }
int getMem(maquinaVirtual *mv, operando op) { return 0; }
int getOp(maquinaVirtual *mv, operando op) { return 0; }
void setOp(maquinaVirtual *mv, operando op, int val) {}
void recuperaOperandos(maquinaVirtual *mv, operando *o, int ip) {}