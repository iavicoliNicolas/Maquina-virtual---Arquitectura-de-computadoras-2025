#ifndef VMI_H
#define VMI_H

#include "mv.h"   // porque usa la estructura maquinaVirtual

int guardarVMI(maquinaVirtual *mv, const char *nombreArchivo);
int leerVMI(maquinaVirtual *mv, const char *nombreArchivo);

#endif
