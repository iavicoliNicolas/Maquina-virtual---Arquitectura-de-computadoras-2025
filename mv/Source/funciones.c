#include "funciones.h"
#include "operando.h"
#include "mv.h"
void cargaVF(Toperaciones *v){
    //operaciones de 2 operandos
    v[0x10] = MOV;
    v[0x11] = ADD;
    v[0x12] = SUB;
    v[0x13] = MUL;
    v[0x14] = DIV;
    v[0x15] = CMP;
    v[0x16] = SHL;
    v[0x17] = SHR;
    v[0x18] = SAR;
    v[0x19] = AND;
    v[0x1A] = OR;
    v[0x1B] = XOR;
    v[0x1C] = SWAP;
    v[0x1D] = LDL;
    v[0x1E] = LDH;
    v[0x1F] = RND;
    //operaciones de 1 operandos
    v[0x00] = SYS;
    v[0x01] = JMP;
    v[0x02] = JZ;
    v[0x03] = JP;
    v[0x04] = JN;
    v[0x05] = JNZ;
    v[0x06] = JNP;
    v[0x07] = JNN;
    v[0x08] = NOT;
    //operaciones sin operandos
    v[0x0F] = STOP;

}


void loadSYSOperationArray(funcionSys *vecLlamadas){
    vecLlamadas[0] = NULL; //no hay operacion 0
    vecLlamadas[1] = readSys;
    vecLlamadas[2] = writeSys;
}

void setLAR(maquinaVirtual *mv, int segmento, int desplaz) {
    mv->registros[LAR] = ((segmento & 0xFFFF) << 16) | (desplaz & 0xFFFF);
}
void setMAR(maquinaVirtual *mv, int nbytes, int dirFisica) {
    mv->registros[MAR] = ((nbytes & 0xFFFF) << 16) | (dirFisica & 0xFFFF);
}
void setMBR(maquinaVirtual *mv, int valor) {
    mv->registros[MBR] = valor;
}
void setRegOP(maquinaVirtual *mv, int reg, operando valor, int tipo) {

    mv->registros[reg] = tipo << 24; //almacenar el tipo en los 8 bits mas significativos
    switch (tipo) {
        case 0: //no usado
            mv->registros[reg] |= 0;
            break;
        case 1: //registro
            mv->registros[reg] |= (valor.registro & 0xFF);
            break;
        case 2: //inmediato
            mv->registros[reg] |= (valor.desplazamiento & 0xFFFF);
            break;
        case 3: //memoria
            mv->registros[reg] |= (valor.registro & 0xFF) << 16;
            mv->registros[reg] |= (valor.desplazamiento & 0xFFFF);
            break;
        default:
            fprintf(stderr, "Error: Tipo de operando invalido: %d\n", tipo);
            exit(EXIT_FAILURE);
    }
}

void setCC(maquinaVirtual *mv, int resultado) {
    if (resultado == 0) {
        mv->registros[CC] = 0b0100000000000000; // Cero
    } else if (resultado > 0) {
        mv->registros[CC] = 0b0000000000000000; // Positivo
    } else {
        mv->registros[CC] = 0b1000000000000000; // Negativo
    }
}


//funciones assembler
void MOV(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[1]));
}
void ADD(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) + getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void SUB(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) - getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void MUL(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) * getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[1]));
}
void DIV(maquinaVirtual *mv, int *op) {
    int aux = getOp(mv, op[1]);
    printf("---->divisor=%d",aux);///--------------------------------------------------------------
    if ( aux == 0) {
        fprintf(stderr, "Error: Division por cero\n");
        exit(EXIT_FAILURE);
    }
    setOp(mv, op[0], getOp(mv, op[0]) / getOp(mv, op[1]));
    mv->registros[AC] = getOp(mv, op[0]) % getOp(mv, op[1]); //guardar el resto en AC
    setCC(mv, getOp(mv, op[0]));
}

void CMP(maquinaVirtual *mv, int *op){
    int resultado = getOp(mv, op[1]) - getOp(mv, op[0]);
    setCC(mv, resultado);
}
void SHL(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[1]) << getOp(mv, op[0]));
    setCC(mv, getOp(mv, op[1]));
}
void SAR(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], getOp(mv, op[0]) >> getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void SHR(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (unsigned int)getOp(mv, op[0]) >> getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void AND(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) & getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void OR(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) | getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void XOR(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], getOp(mv, op[0]) ^ getOp(mv, op[1]));
    setCC(mv, getOp(mv, op[0]));
}
void SWAP(maquinaVirtual *mv, int *op){
    int temp = getOp(mv, op[1]);
    setOp(mv, op[1], getOp(mv, op[0]));
    setOp(mv, op[0], temp);
}
void RND(maquinaVirtual *mv, int *op){
    setOp(mv, op[1], rand());
}
void SYS(maquinaVirtual *mv, int *op) {
    // Obtener el número de syscall del operando inmediato
    int llamada = getOp(mv, op[0]); // Asumo que getOp devuelve el número

    funcionSys vecLlamadas[3];
    loadSYSOperationArray(vecLlamadas);

    if (llamada >= 1 && llamada <= 2) {
        vecLlamadas[llamada](mv, llamada);
    } else {
        fprintf(stderr, "Error: Llamada al sistema no válida: %d\n", llamada);
        exit(EXIT_FAILURE);
    }
}
void JMP(maquinaVirtual *mv, int *op){
    mv->registros[IP] = getOp(mv, op[0]);
}
void JZ(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] == 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JP(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] > 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JN(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] < 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNZ(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] != 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNP(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] >= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void JNN(maquinaVirtual *mv, int *op){
    if (mv->registros[CC] <= 0) {
        mv->registros[IP] = getOp(mv, op[0]);
    }
}
void LDL(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFFFF00) | (getOp(mv, op[1]) & 0x000000FF));
}
void LDH(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFF00FF) | ((getOp(mv, op[1]) & 0x000000FF) << 8));
}
void NOT(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], ~getOp(mv, op[0]));
}
void STOP(maquinaVirtual *mv, int *op){
    exit(EXIT_SUCCESS);
}
void readSys(maquinaVirtual *mv, int arg) {
    // Obtener parámetros de los registros según la documentación
    int direccion_edx = mv->registros[EDX];
    int modo_eax = mv->registros[EAX] & 0xFF; // Modo en AL (byte bajo de EAX)

    // ECX: bytes menos significativos = cantidad de celdas (CL)
    //       bytes más significativos = tamanio de celdas (CH)
    int cantidad_celdas = mv->registros[ECX] & 0xFF;        // CL
    int tamanio_celda = (mv->registros[ECX] >> 8) & 0xFF;    // CH

    // Convertir dirección lógica a física
    int dir_fisica_base = logicoAFisico(mv, direccion_edx);

    // Procesar cada celda
    int i;
    for (i = 0; i < cantidad_celdas; i++) {
        int dir_actual = dir_fisica_base + (i * tamanio_celda);

        // Mostrar prompt con dirección física (4 dígitos hex)
        printf("[%04X]: ", dir_actual);

        // Leer según el modo habilitado
        if (modo_eax & 0x01) { // Decimal (bit 0)
            leerYAlmacenarDecimal(mv, dir_actual, tamanio_celda);
        }
        else if (modo_eax & 0x02) { // Caracteres (bit 1)
            leerYAlmacenarCaracter(mv, dir_actual, tamanio_celda);
        }
        else if (modo_eax & 0x04) { // Octal (bit 2)
            leerYAlmacenarOctal(mv, dir_actual, tamanio_celda);
        }
        else if (modo_eax & 0x08) { // Hexadecimal (bit 3)
            leerYAlmacenarHexadecimal(mv, dir_actual, tamanio_celda);
        }
        else if (modo_eax & 0x10) { // Binario (bit 4)
            leerYAlmacenarBinario(mv, dir_actual, tamanio_celda);
        }
        else {
            // Por defecto, decimal
            leerYAlmacenarDecimal(mv, dir_actual, tamanio_celda);
        }
    }
}

// Funciones auxiliares específicas para cada modo
void leerYAlmacenarDecimal(maquinaVirtual *mv, int dir, int tamanio) {
    int valor;
    scanf("%d", &valor);
    almacenarValorEnMemoria(mv, dir, tamanio, valor);
}

void leerYAlmacenarCaracter(maquinaVirtual *mv, int dir, int tamanio) {
    char c;
    scanf(" %c", &c); // El espacio ignora whitespace anterior
    // Para caracteres, solo usamos el primer byte, el resto se llena con 0
    mv->memoria[dir] = c;
    int i;
    for (i = 1; i < tamanio; i++) {
        mv->memoria[dir + i] = 0;
    }
}

void leerYAlmacenarHexadecimal(maquinaVirtual *mv, int dir, int tamanio) {
    unsigned int valor;
    scanf("%x", &valor);
    almacenarValorEnMemoria(mv, dir, tamanio, (int)valor);
}

void leerYAlmacenarOctal(maquinaVirtual *mv, int dir, int tamanio) {
    unsigned int valor;
    scanf("%o", &valor);
    almacenarValorEnMemoria(mv, dir, tamanio, (int)valor);
}

void leerYAlmacenarBinario(maquinaVirtual *mv, int dir, int tamanio) {
    char bin_str[33];
    scanf("%32s", bin_str); // Máximo 32 bits
    int valor = (int)strtol(bin_str, NULL, 2);
    almacenarValorEnMemoria(mv, dir, tamanio, valor);
}

// Función genérica para almacenar valores en memoria (big-endian)
void almacenarValorEnMemoria(maquinaVirtual *mv, int dir, int tamanio, int valor) {
    // Almacenar en big-endian (más significativo primero)
    int i;
    for (i = 0; i < tamanio; i++) {
        int shift = (tamanio - 1 - i) * 8;
        mv->memoria[dir + i] = (valor >> shift) & 0xFF;
    }
}

void writeSys(maquinaVirtual *mv, int arg) {

    // Obtener parámetros de los registros
    int direccion_edx = mv->registros[EDX];
    int modo_eax = mv->registros[EAX];
    int cantidad_celdas = mv->registros[ECX] & 0xFFFF;
    int tamanio_celda = (mv->registros[ECX] >> 8) & 0xFFFF;

    // Convertir dirección lógica a física
    int dir_fisica_base = logicoAFisico(mv, direccion_edx);

    // Escribir cada celda
    int i;
    for (i = 0; i < cantidad_celdas; i++) {
        int dir_actual = dir_fisica_base + (i * tamanio_celda);

        // Mostrar prompt con dirección física
        printf("[%04X]: ", dir_actual);

        // Leer valor de memoria (big-endian)
        int valor = 0;
        int j;
        for (j = 0; j < tamanio_celda; j++) {
            valor = (valor << 8) | (mv->memoria[dir_actual + j] & 0xFF);
        }

        // Mostrar según los modos habilitados
        int modos_mostrados = 0;

        if (modo_eax & 0x10) { // Binario (bit 4)
            mostrarBinario(valor, tamanio_celda);
            modos_mostrados++;
        }
        if (modo_eax & 0x08) { // Hexadecimal (bit 3)
            if (modos_mostrados > 0) printf(" ");
            printf("0x%0*X", tamanio_celda * 2, valor);
            modos_mostrados++;
        }
        if (modo_eax & 0x04) { // Octal (bit 2)
            if (modos_mostrados > 0) printf(" ");
            printf("0%0*o", (tamanio_celda * 3) + 1, valor);
            modos_mostrados++;
        }
        if (modo_eax & 0x02) { // Caracteres (bit 1)
            if (modos_mostrados > 0) printf(" ");
            if (valor >= 32 && valor <= 126) {
                printf("'%c'", (char)valor);
            } else {
                printf(".");
            }
            modos_mostrados++;
        }
        if (modo_eax & 0x01) { // Decimal (bit 0)
            if (modos_mostrados > 0) printf(" ");
            printf("%d", valor);
            modos_mostrados++;
        }

        printf("\n");
    }
}

void mostrarBinario(int valor, int tamanio) {
    int bits = tamanio * 8;
    int i;
    for (i = bits - 1; i >= 0; i--) {
        printf("%d", (valor >> i) & 1);
        if (i % 4 == 0 && i > 0) printf(" ");
    }
}
