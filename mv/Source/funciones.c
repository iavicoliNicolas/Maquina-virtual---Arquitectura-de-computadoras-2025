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
    v[0x0B] = PUSH;
    v[0x0C] = POP;
    v[0x0D] = CALL;
    v[0x0E] = RET;
    //operaciones sin operandos
    v[0x0F] = STOP;

}


void loadSYSOperationArray(funcionSys *vecLlamadas){
    vecLlamadas[0] = NULL; //no hay operacion 0
    vecLlamadas[1] = readSys;
    vecLlamadas[2] = writeSys;
}

void setLAR(maquinaVirtual *mv, int dirL) {
    mv->registros[LAR] = dirL;
    printf("LAR seteado a: 0x%08X\n", mv->registros[LAR]);
}
void setMAR(maquinaVirtual *mv, int nbytes, int dirFisica) {
    mv->registros[MAR] = ((nbytes & 0xFFFF) << 16) | (dirFisica & 0xFFFF);
    printf("MAR seteado a: 0x%08X\n", mv->registros[MAR]);
}
void setMBR(maquinaVirtual *mv, int valor) {
    mv->registros[MBR] = valor;
    printf("MBR seteado a: 0x%08X\n", mv->registros[MBR]);
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

void setCC(maquinaVirtual *mv, int numero) {
    int cc = mv->registros[17];

    // limpiar bits N (31) y Z (30)
    cc &= 0x3FFFFFFF;   // 0011 1111 1111 1111 1111 1111 1111 1111

    if (numero == 0) {
        cc |= 0x40000000;  // bit Z = 1
    }
    else if (numero < 0) {
        cc |= 0x80000000;  // bit N = 1
    }
    // si numero > 0 → N=0, Z=0 (no se marca nada)

    mv->registros[17] = cc;
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
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFF0000) | (getOp(mv, op[1]) & 0x0000FFFF));
}
void LDH(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], (getOp(mv, op[0]) & 0xFFFF0000) | ((getOp(mv, op[1]) & 0x0000FFFF) << 16));
}
void NOT(maquinaVirtual *mv, int *op){
    setOp(mv, op[0], ~getOp(mv, op[0]));
}
void PUSH(maquinaVirtual *mv, int *op){ //COMPLETAR/ARREGLAR
    
    int aux;
    int posSeg = (mv->registros[SP] >> 16) & 0x0000000F;
    int offset = mv->registros[SP] & 0x0000FFFF;

    mv->registros[SP] -= 4; // Asumiendo tamaño de palabra de 4 bytes
    if ( mv->memoria[SP] < mv->memoria[SS] ) {
        printf("ERROR: STACK OVERFLOW\n");
        STOP(mv, op);
    } 
    aux = getOp(mv, op[0]);
    mv->memoria[posSeg + offset + 3] = (aux & 0x000000FF);
    mv->memoria[posSeg + offset + 2] = (aux >> 8) & 0x000000FF;
    mv->memoria[posSeg + offset + 1] = (aux >> 16) & 0x000000FF;
    mv->memoria[posSeg + offset] = (aux >> 24) & 0x000000FF;
}
void POP(maquinaVirtual *mv, int *op){//COMPLETAR/ARREGLAR
    
    int aux = 0;
    int posSeg = (mv->registros[SP] >> 16) & 0x0000000F;
    int offset = mv->registros[SP] & 0x0000FFFF;

    if ( mv->tablaSegmentos[posSeg][1] < offset ) {
        printf("ERROR: STACK UNDERFLOW\n");
        STOP(mv, op);
    } else {

        aux |= mv->memoria[posSeg + offset] << 24;
        aux |= (0x00FF0000 & (mv->memoria[posSeg + offset + 1] << 16));
        aux |= (0x0000FF00 & (mv->memoria[posSeg + offset + 2] << 8));
        aux |= (0x000000FF & (mv->memoria[posSeg + offset + 3]));
        setOp(mv, op[0], aux);
    }


    mv->registros[SP] += 4; // Asumiendo tamaño de palabra de 4 bytes
}
void CALL(maquinaVirtual *mv, int *op){//COMPLETAR/ARREGLAR
    
    int aux;

    mv->registros[SP] -= 4; // Asumiendo tamaño de palabra de 4 bytes
    short int puntero = mv->registros[SP] & 0x0000FFFF;
    int posSeg = (mv->registros[SP] >> 16) & 0x0000000F;

    if(mv->registros[SP] < mv->registros[SS]){
        printf("ERROR: STACK OVERFLOW\n");
        STOP(mv,op);
    }else{

        aux = mv->registros[IP];

        mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 3] = (aux & 0x000000FF);
        mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 2] = (aux & 0x0000FF00)>>8;
        mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 1] = (aux & 0x00FF0000)>>16;
        mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero] = (aux & 0xFF000000)>>24;

        mv->registros[IP] = getOp(mv,op[0]);
    }
}
void RET(maquinaVirtual *mv, int *op){//COMPLETAR/ARREGLAR
    
    int aux = 0;
    short int puntero = mv->registros[SP] & 0x0000FFFF;
    int posSeg = (mv->registros[SP] >> 16) & 0x0000000F;

    if(mv->tablaSegmentos[posSeg][1] < puntero){
        printf("ERROR: STACK UNDERFLOW\n");
        STOP(mv,op);
    }else{

        aux |= mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero] << 24;
        aux |= (0x00FF0000 & (mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 1] << 16));
        aux |= (0x0000FF00 & (mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 2] << 8));
        aux |= (0x000000FF & (mv->memoria[mv->tablaSegmentos[posSeg][0] + puntero + 3]));
        mv->registros[IP] = aux;

        mv->registros[SP] += 4; // Asumiendo tamaño de palabra de 4 bytes
    }
}
void STOP(maquinaVirtual *mv, int *op){
    exit(EXIT_SUCCESS);
}

//------------------- Implementación de readSys y writeSys ------------------//

// Función genérica para almacenar valores en memoria (big-endian)
void almacenarValorEnMemoria(maquinaVirtual *mv, int dir, int tamanio, int valor) {
    // Almacenar en big-endian (más significativo primero)
    int i;
    for (i = 0; i < tamanio; i++) {
        int shift = (tamanio - 1 - i) * 8;
        mv->memoria[dir + i] = (valor >> shift) & 0xFF;
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

void mostrarBinario(int valor, int tamanio) {
    int bits = tamanio * 8;
    int i;
    for (i = bits - 1; i >= 0; i--) {
        printf("%d", (valor >> i) & 1);
        if (i % 4 == 0 && i > 0) printf(" ");
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


//------------------- Fin de la implementación de nuevas funciones Sys ------------------//

//------------------- Implementación de nuevas funciones Sys ------------------//

void readStringSys(maquinaVirtual *mv){

    char* st;
    int i = 0;

    gets(st);
    
    while ( i < strlen(st) || i < mv->registros[ECX] ) {

        mv->memoria[mv->registros[EDX] + i] = st[i];
        i++;
    }

}
void writeStringSys(maquinaVirtual *mv, int arg){
    
    char c;

    int i = 0, ok = 1;

    while( ok ){
        
        c = mv->memoria[mv->registros[EDX] + i];
        if(c == '\0'){
            ok = 0;
        }
        printf("%c",c);
        i++;
    }
    printf("\n");
}
void clearScreenSys(maquinaVirtual *mv, int arg){
    // Lógica para limpiar la pantalla
    //system("clear"); // En sistemas Unix/Linux
    system("cls"); // En sistemas Windows
}
void breakPointSys(maquinaVirtual *mv, int arg) {

    
}

void generaArchivoDeImagen(maquinaVirtual mv) {

    FILE *arch;
    unsigned short int i, tamanio;

    
}

//------------------- Fin de la implementación de nuevas funciones Sys ------------------//