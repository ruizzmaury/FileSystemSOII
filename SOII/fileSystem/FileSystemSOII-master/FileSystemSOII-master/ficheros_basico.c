#include "ficheros_basico.h"
#include <limits.h>

/* 
tamMB function
Computes the size in blocks required for the bit map.

*/
int tamMB(unsigned int nbloques)
{
    int bytes = nbloques / 8;       // number of bytes required
    int blocks = bytes / BLOCKSIZE; // bytes grouped in blocks with size: 'BLOCKSIZE'

    if (blocks % BLOCKSIZE != 0) // add extra block if the remainder is != of 0
    {                            // because there're missing bytes to group
        return blocks++;
    }
    else
    {
        return blocks;
    }
}

/* 
tamAI function
Computes the size in blocks of the inode array.

*/
int tamAI(unsigned int ninodos)
{

    int tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;

    if (tamAI % BLOCKSIZE != 0) // add extra block if the remainder is != of 0
    {                           // because there're missing bytes to group
        return tamAI++;
    }
    else
    {
        return tamAI;
    }
}

/* 
initMB function
Initialize Bit MAP

*/
int initMB()
{
    // creating buffer
    unsigned char buff[BLOCKSIZE];

    memset(buff, 0, BLOCKSIZE);

    struct superbloque SB;

    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    for (int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueAI; i++)
    {
        if (bwrite(i, buff) == 1)
        {
            fprintf(stderr, "Write error on the device. \n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

/* 
initSB function
Initialize superblock data

*/
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;

    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB = 1

    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;

    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;

    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;

    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;

    SB.posUltimoBloqueDatos = nbloques - 1;

    SB.posInodoRaiz = 0;

    SB.posPrimerInodoLibre = 0;

    SB.cantBloquesLibres = nbloques;

    SB.cantInodosLibres = ninodos;

    SB.totBloques = nbloques;

    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == 1)
    {
        fprintf(stderr, "Write error on the device. \n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int initAI()
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    int continodos = SB.posPrimerInodoLibre + 1;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Itera dentro de cada estructura de inodos.
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            // Iniciliza el contenido del inodo.
            inodos[j].tipo = 'l';
            if (continodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = continodos;
                continodos++;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
            }
        }
        // write inodos block into the virtual device
        if (bwrite(i, inodos) == 1)
        {
            fprintf(stderr, "Write error on the device. \n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{

    struct superbloque SB;

    // read superblock to get the MB location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE]; // buffer creation
    posbyte = posbyte % BLOCKSIZE;     // position byte inside block

    // read the block which contains the specific bit
    if (bread(nbloqueabs, &bufferMB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha

    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara; //  operador OR para bits
    }
    else
    {
        bufferMB[posbyte] &= ~mascara; // operadores AND y NOT para bits
    }

    // write MB buffer into the virtual device
    if (bwrite(nbloqueabs, bufferMB) == 1)
    {
        fprintf(stderr, "Write error on the device. \n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;

    // read superblock to get the MB location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE]; // buffer creation
    posbyte = posbyte % BLOCKSIZE;     // position byte inside block

    // read the block which contains the specific bit
    if (bread(nbloqueabs, &bufferMB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    unsigned char mascara = 128;  // 10000000
    mascara >>= posbit;           // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha

    return mascara;
}

int reservar_bloque()
{

    struct superbloque SB;

    // read superblock to get the MB location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    if (SB.cantBloquesLibres <= 0)
    {
        return EXIT_FAILURE;
    }

    // buffer creation
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con 1s

    for (int posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB < SB.posUltimoBloqueMB; posBloqueMB++)
    {
        if (bread(posBloqueMB, bufferMB) == 1)
        {
            fprintf(stderr, "Read error on the device. \n");
            return EXIT_FAILURE;
        }

        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) < 0)
        {
            // locate wich byte of the block has some 0
            break;
        }
    }

    // in bufferMB
    // we already got the block where there is at least one 0 in a byte
    // now we are finding the first byte with a 0
    int posByte = 0; // future position of the first byte in the block with a 0
    for (posByte; posByte < BLOCKSIZE; posByte++)
    {
        if (bufferMB[posByte] != 255)
        {
            break;
        }
    }

    unsigned char mascara = 128; // 10000000
    unsigned int posBloqueMB = SB.posPrimerBloqueMB;
    unsigned int posbit = 0;

    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posByte] & mascara)
    {                            // operador AND para bits
        bufferMB[posByte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }

    int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posByte) * 8 + posbit;

    //  reserve block in bitmap MB
    if (escribir_bit(nbloque, 1))
    {
        return EXIT_FAILURE;
    }

    SB.cantBloquesLibres--; // reducing free blocks number

    // cleaning the nbloque position
    unsigned char cleanerBuff[BLOCKSIZE];

    memset(cleanerBuff, 0, BLOCKSIZE); // Fillin up the buffer with 0s

    // writing those 0s in the correct location
    if (bwrite(nbloque, &cleanerBuff) == 1)
    {
        fprintf(stderr, "Writing error. \n");
        return EXIT_FAILURE;
    }

    return nbloque;
}

int liberar_bloque(unsigned int nbloque)
{

    struct superbloque SB;

    // read superblock to get the MB location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    if (escribir_bit(nbloque, 0))
    {
        return EXIT_FAILURE;
    }

    SB.cantBloquesLibres++; // increasing number of free blocks

    // write SB into the disk
    if (bwrite(posSB, &SB) == 1)
    {
        fprintf(stderr, "Write error on the device. \n");
        return EXIT_FAILURE;
    }

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct superbloque SB;

    // read superblock to get the AI location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE]; // buffer creation

    // read the block which contains the specific inodo
    if (bread((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))),
              &inodos) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    // write inode into the arrayInodos
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo;

    // write modified block into the virtual device
    if (bwrite((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))), &inodos) == 1)
    {
        fprintf(stderr, "Write error on the device. \n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{

    struct superbloque SB;

    // read superblock to get the AI location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE]; // buffer creation

    // read the block which contains the specific inodo
    if (bread((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))),
              &inodos) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    // set the correct inodo
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXIT_SUCCESS;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodo;

    // read superblock to get the AI location
    if (bread(posSB, &SB) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    // checking if we still free inodos
    if (SB.cantInodosLibres <= 0)
    {
        return EXIT_FAILURE;
    }

    // keeping the initial free inodo position
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;

    // reading the first free inodo
    if (leer_inodo(posInodoReservado, &inodo) == 1)
    {
        fprintf(stderr, "Reading error. \n");
        return EXIT_FAILURE;
    }

    // making the second free inodo position the first inodo position of the SB
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];

    // initializing the fields of the initial free inodo of the SB
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.numBloquesOcupados = 0;
    // initializing to 0 the 12's pointers of both types direct and indirect
    for (int i = 0; i < (sizeof(inodo.punterosDirectos) / sizeof(inodo.punterosDirectos[0])); i++)
    {
        inodo.punterosDirectos[i] = 0;
    }

    for (int i = 0; i < (sizeof(inodo.punterosIndirectos) / sizeof(inodo.punterosIndirectos[0])); i++)
    {
        inodo.punterosIndirectos[i] = 0;
    }

        // writing the inodo initialized in the position which
    // was initially the first free inodo
    if (escribir_inodo(posInodoReservado, inodo) == 1)
    {
        fprintf(stderr, "Writing inodo error. \n");
        return EXIT_FAILURE;
    }
    // updating the number of free inodos of the SB
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == -1)
    {
        return EXIT_FAILURE;
    }

    return posInodoReservado;
}

int obtener_nRangoBL (struct inodo inodo, unsigned int nblogico, int *ptr) {
     if (nblogico < DIRECTOS)
    {
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    }
    if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    }
    if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    }
    if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    }

    // If it is out of range then raise Returns Failure
    *ptr = 0;
    return EXIT_FAILURE;
}

int obtener_indice(unsigned int nblogico, unsigned int nivel_punteros)
{
    // if the logic block is located in directos -> returns its position
    if (nblogico < DIRECTOS) 
    {
        return nblogico; //ej nblogico=8
    }

    // if the logic block is located in indirectos 0 -> To this, subtract the directos
    if (nblogico < INDIRECTOS0) 
    {
        return nblogico - DIRECTOS;  //ej nblogico=204
    }

    /* */
    if (nblogico < INDIRECTOS1)  //ej nblogico=30.004    
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }

    /* */
    if (nblogico < INDIRECTOS2) //ej nblogico=400.004
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) /
                   NPUNTEROS;
        }
        if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) %
                   NPUNTEROS;
        }
    }

    // If something wrong happens then raise Returns Failure
    return EXIT_FAILURE;
}


int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar){
   unsigned int ptr = 0;
   // leemos el inodo solicitado.
   struct inodo inodo;
   leer_inodo(ninodo, &inodo);
   if (nblogico < DIRECTOS){ // el bloque logico es uno de los 12 primeros bloques logicos del inodo.
       switch (reservar){
           case 0:// modo consulta
               if (inodo.punterosDirectos[nblogico] == 0)// no tiene bloque físico asignado.
                   return -1;
               else{
                   ptr=inodo.punterosDirectos[nblogico];
               }
               break;
           case 1:// modo escritura
               if (inodo.punterosDirectos[nblogico] == 0){// si no tiene bloque fisico le asignamos uno.
                   inodo.punterosDirectos[nblogico] = reservar_bloque();
                   ptr=inodo.punterosDirectos[nblogico];
                  // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos:
                   inodo.numBloquesOcupados++;
                   inodo.ctime = time(NULL);
                   // escribimos el inodo con la info actualizada
                   escribir_inodo(ninodo,inodo);
               }
               else{ // tiene bloque fisico asignado y lo devolvemos
                   ptr=inodo.punterosDirectos[nblogico];
               }
               break;
       }
       //printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
       return ptr;
   }
   // PUNTERO INDIRECTOS 0
   // El bloque logico lo encontramos en el rango de Indirectos 0, es decir, 
   // está comprendido entre   el 0+12 y el 0+12+256-1: entre el 12 y el 267 
   else if (nblogico < INDIRECTOS0){
       unsigned int punteros_nivel1[NPUNTEROS];
       switch (reservar){
           case 0:// modo consulta
               if (inodo.punterosIndirectos[0] == 0)// no hay bloque fisico asignado a punteros_nivel1.
                   return -1;
               else{// ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[0],punteros_nivel1);
                   if (punteros_nivel1[nblogico-DIRECTOS] == 0)
                   // no hay bloque físico asignado al bloque lógico de datos.
                       return -1;
                   else{
                       ptr=punteros_nivel1[nblogico-DIRECTOS];
                   }
               }
               break;
           case 1:// modo escritura
               if (inodo.punterosIndirectos[0] == 0){
               // no hay bloque fisico asignado a punteros_nivel1, asi que creamos un buffer con 0s,  y
               // reservamos un bloque donde salvarlo.
                   memset(punteros_nivel1,0,BLOCKSIZE);//iniciamos a 0 los 256 punteros.
                   inodo.punterosIndirectos[0] = reservar_bloque();
                   // reservamos un nuevo bloque para los datos y anotamos su nº 
                   // en el índice correspondiente de punteros_nivel1 
                   punteros_nivel1[nblogico-DIRECTOS] = reservar_bloque();
                   // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                   inodo.numBloquesOcupados+=2;
                   inodo.ctime = time(NULL);
                   bwrite(inodo.punterosIndirectos[0],punteros_nivel1);
                   // devolvemos el bloque fisico de datos
                   ptr=punteros_nivel1[nblogico-DIRECTOS];
               }              
               else{// existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[0],punteros_nivel1);
                   if (punteros_nivel1[nblogico-DIRECTOS] == 0){
                       // no hay bloque fisico de datos asignado, entonces lo reservamos
                       punteros_nivel1[nblogico-DIRECTOS] = reservar_bloque();
                       // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                       inodo.numBloquesOcupados++;
                       inodo.ctime = time(NULL);
                       // salvamos el bloque de punteros_nivel1 en el dispositivo.
                       bwrite(inodo.punterosIndirectos[0],punteros_nivel1);
                       ptr=punteros_nivel1[nblogico-DIRECTOS]; // devolvemos el bloque fisico de datos.
                   }
                   else{ // si existe el bloque fisico de datos lo devolvemos
                       ptr=punteros_nivel1[nblogico-DIRECTOS];
                   }
               }
               // escribimos en el dispositivo el inodo actualizado
               escribir_inodo(ninodo,inodo);
               break;
       }
       //printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
       return ptr;
   }
   // PUNTERO INDIRECTOS 1
   // El bloque logico lo encontramos en el rango de Indirectos 1, es decir, 
   // los comprendidos entre el 0+12+256 y el 0+12+256+256^2-1: entre el 268 y el 65.803.
   else if (nblogico < INDIRECTOS1){
       unsigned int punteros_nivel1[NPUNTEROS];
       unsigned int punteros_nivel2[NPUNTEROS];
       unsigned int indice_nivel1 = obtener_indice(nblogico, 1); // indice para punteros_nivel1
       unsigned int indice_nivel2 = obtener_indice(nblogico, 2); // indice para punteros_nivel2
       switch(reservar){
           case 0:// modo consulta
               if (inodo.punterosIndirectos[1] == 0)// no hay bloque fisico asignado a punteros_nivel2.
                   return -1;
               else{// ya existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[1],punteros_nivel2);
                   if (punteros_nivel2[indice_nivel2] == 0) {// no hay bloque fisico asignado a punteros_nivel1.
                       return -1;
                   } else {// ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                       bread(punteros_nivel2[indice_nivel2],punteros_nivel1);
                       if (punteros_nivel1[indice_nivel1] == 0)
                        // no hay bloque físico asignado al bloque lógico de datos.
                           return -1;
                       else{
                           ptr=punteros_nivel1[indice_nivel1]; // devolvemos el bloque fisico solicitado
                       }
                   }
               }
               break;
           case 1:// modo escritura
               if (inodo.punterosIndirectos[1] == 0){
               // no hay bloque fisico asignado a punteros_nivel2, asi que llenamos un buffer con 0s,  y
               // reservamos un bloque donde salvarlo. 
                   memset(punteros_nivel2,0,BLOCKSIZE);
                   inodo.punterosIndirectos[1] = reservar_bloque(); 
                   // llenamos el buffer de punteros_nivel1 con 0s y reservamos un bloque para él, 
     // anotamos su nº  en el índice correspondiente de punteros_nivel2 
                   memset(punteros_nivel1,0,BLOCKSIZE);
                   punteros_nivel2[indice_nivel2] = reservar_bloque(); 
                   // reservamos un nuevo bloque para los datos y anotamos su nº 
                   // en el índice correspondiente de punteros_nivel1 
                   punteros_nivel1[indice_nivel1] = reservar_bloque(); 
                   // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                   inodo.numBloquesOcupados+=3;
                   inodo.ctime = time(NULL);
                   // salvamos los buffers de los bloques de punteros en el dispositivo
                   bwrite(inodo.punterosIndirectos[1],punteros_nivel2);
                   bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                   // devolvemos el bloque fisico de datos
                   ptr=punteros_nivel1[indice_nivel1];
               }
               else{// existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[1],punteros_nivel2);
                   if (punteros_nivel2[indice_nivel2] == 0){ 
                   // no hay bloque fisico de punteros_nivel1 asignado, entonces lo reservamos y
     // anotamos su nº  en el índice correspondiente de punteros_nivel2
                       memset(punteros_nivel1,0,BLOCKSIZE);
                       punteros_nivel2[indice_nivel2] = reservar_bloque();   
                       // reservamos un nuevo bloque para los datos y anotamos su nº 
                       // en el índice correspondiente de punteros_nivel1 
                       punteros_nivel1[indice_nivel1] = reservar_bloque(); 
                       // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                       inodo.numBloquesOcupados+=2;
                       inodo.ctime = time(NULL);
                       // salvamos los buffers de los bloques de punteros en el dispositivo
                       bwrite(inodo.punterosIndirectos[1],punteros_nivel2);
                       bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                       // devolvemos el bloque fisico de datos
                       ptr=punteros_nivel1[indice_nivel1];
                   }
                   else{// existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                       bread(punteros_nivel2[indice_nivel2],punteros_nivel1);    
                       if (punteros_nivel1[indice_nivel1] == 0){
                       // no hay bloque físico asignado al bloque lógico de datos así que lo reservamos 
                       // y anotamos su nº en el índice correspondiente de punteros_nivel1.
                           punteros_nivel1[indice_nivel1] = reservar_bloque();
                           // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
                           inodo.numBloquesOcupados++;
                           inodo.ctime = time(NULL);
                           // salvamos el bloque de punteros_nivel1 en el dispositivo
                           bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                           // devolvemos el bloque físico asignado a los datos
                           ptr = punteros_nivel1[indice_nivel1];
                       }
                       else{
                           ptr = punteros_nivel1[indice_nivel1];
                       }
                   }
               }
               // escribimos en el dispositivo el inodo actualizado
               escribir_inodo(ninodo,inodo);
               break;
       }
       //printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
       return ptr;
   }
   // PUNTERO INDIRECTOS 2
   // El bloque logico lo encontramos en el rango de Indirectos 2, es decir, los comprendidos entre 
   // el 0+12+256+256^2 y el 0+12+256+256^2+256^3-1: entre el 65.804 y el 16.843.019.
   else if (nblogico < INDIRECTOS2){
       unsigned int punteros_nivel1[NPUNTEROS];
       unsigned int punteros_nivel2[NPUNTEROS];
       unsigned int punteros_nivel3[NPUNTEROS];
       unsigned int indice_nivel1 = obtener_indice(nblogico, 1); //indice para punteros_nivel1
       unsigned int indice_nivel2 = obtener_indice(nblogico, 2); //indice para punteros_nivel2
       unsigned int indice_nivel3 = obtener_indice(nblogico, 3); //indice para punteros_nivel3
       switch(reservar){
           case 0://modo consulta
               if (inodo.punterosIndirectos[2] == 0)// no hay bloque fisico asignado a punteros_nivel3.
                   return -1;
               else{// ya existe el bloque de punteros_nivel3 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[2],punteros_nivel3);
                   if (punteros_nivel3[indice_nivel3] == 0)// no hay bloque fisico asignado a punteros_nivel2.
                    return -1;
                   else{// ya existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                       bread(punteros_nivel3[indice_nivel3],punteros_nivel2);
                       if (punteros_nivel2[indice_nivel2] == 0)// no hay bloque fisico asignado a punteros_nivel1.
                           return -1;
                       else{// ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                           bread(punteros_nivel2[indice_nivel2],punteros_nivel1);
                           if (punteros_nivel1[indice_nivel1] == 0)
                           // no hay bloque físico asignado al bloque lógico de datos.
                               return -1;
                           else{
                               ptr=punteros_nivel1[indice_nivel1];// devolvemos el bloque fisico solicitado
                           }
                       }
                   }
               }
               break;
           case 1://modo escritura
               if (inodo.punterosIndirectos[2] == 0){               
               // no hay bloque fisico asignado a punteros_nivel3, asi que llenamos un buffer con 0s,  y
               // reservamos un bloque donde salvarlo. 
                   memset(punteros_nivel3,0,BLOCKSIZE);
                   inodo.punterosIndirectos[2] = reservar_bloque();
                    // llenamos el buffer de punteros_nivel2 con 0s y reservamos un bloque para él, 
      //anotamos su nº  en el índice correspondiente de punteros_nivel3 
                   memset(punteros_nivel2,0,BLOCKSIZE);
                   punteros_nivel3[indice_nivel3] = reservar_bloque();
                   // llenamos el buffer de punteros_nivel1 con 0s y reservamos un bloque para él, 
      //anotamos su nº  en el índice correspondiente de punteros_nivel2 
                   memset(punteros_nivel1,0,BLOCKSIZE);
                   punteros_nivel2[indice_nivel2] = reservar_bloque();
                   // reservamos un nuevo bloque para los datos y anotamos su nº 
                   // en el índice correspondiente de punteros_nivel1 
                   punteros_nivel1[indice_nivel1] = reservar_bloque();
                   // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
                   inodo.numBloquesOcupados+=4;
     inodo.ctime = time(NULL);
                   // salvamos los buffers de los bloques de punteros en el dispositivo
                   bwrite(inodo.punterosIndirectos[2],punteros_nivel3);
                   bwrite(punteros_nivel3[indice_nivel3],punteros_nivel2);
                   bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                   // devolvemos el bloque fisico de datos
                   ptr=punteros_nivel1[indice_nivel1];
               }
               else{// existe el bloque de punteros_nivel3 y lo leemos del dispositivo
                   bread(inodo.punterosIndirectos[2],punteros_nivel3);
                   if (punteros_nivel3[indice_nivel3] == 0){
                   // no hay bloque fisico de punteros_nivel2 asignado, entonces lo reservamos y
     // anotamos su nº  en el índice correspondiente de punteros_nivel3
     memset(punteros_nivel2,0,BLOCKSIZE);
                       punteros_nivel3[indice_nivel3] = reservar_bloque();     
                       // llenamos el buffer de punteros_nivel1 con 0s y reservamos un bloque para él, 
         // anotamos su nº  en el índice correspondiente de punteros_nivel2 
                       memset(punteros_nivel1,0,BLOCKSIZE);
                       punteros_nivel2[indice_nivel2] = reservar_bloque(); 
                       // reservamos un nuevo bloque para los datos y anotamos su nº 
                       // en el índice correspondiente de punteros_nivel1 
                       punteros_nivel1[indice_nivel1] = reservar_bloque(); 
                       // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                       inodo.numBloquesOcupados+=3;
                       inodo.ctime = time(NULL);
                       // salvamos los buffers de los bloques de punteros en el dispositivo
                       bwrite(inodo.punterosIndirectos[2],punteros_nivel3);
                       bwrite(punteros_nivel3[indice_nivel3],punteros_nivel2);
                       bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                       // devolvemos el bloque fisico de datos
                       ptr=punteros_nivel1[indice_nivel1];
                   }
                   else{// existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                       bread(punteros_nivel3[indice_nivel3],punteros_nivel2);
                       if (punteros_nivel2[indice_nivel2] == 0){
                       // no hay bloque fisico de punteros_nivel1 asignado, entonces lo reservamos y
                       // anotamos su nº  en el índice correspondiente de punteros_nivel2
                           memset(punteros_nivel1,0,BLOCKSIZE);
                           punteros_nivel2[indice_nivel2] = reservar_bloque(); 
                           // reservamos un nuevo bloque para los datos y anotamos su nº 
                           // en el índice correspondiente de punteros_nivel1 
                           punteros_nivel1[indice_nivel1] = reservar_bloque();                            
                           // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                           inodo.numBloquesOcupados+=2;
                           inodo.ctime = time(NULL);
                           // salvamos los buffers de los bloques de punteros en el dispositivo
                           bwrite(punteros_nivel3[indice_nivel3],punteros_nivel2);
                           bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                            // devolvemos el bloque fisico de datos
                           ptr=punteros_nivel1[indice_nivel1];
                       }
                       else{// existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                           bread(punteros_nivel2[indice_nivel2],punteros_nivel1);
                           if (punteros_nivel1[indice_nivel1] == 0){
                           // no hay bloque físico asignado al bloque lógico de datos así que lo reservamos 
                           // y anotamos su nº en el índice correspondiente de punteros_nivel1.
                               punteros_nivel1[indice_nivel1] = reservar_bloque();                                
                               //aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                               inodo.numBloquesOcupados++;
                               inodo.ctime = time(NULL);
                               // salvamos el bloque de punteros_nivel1 en el dispositivo
                               bwrite(punteros_nivel2[indice_nivel2],punteros_nivel1);
                               // devolvemos el bloque físico asignado a los datos
                               ptr=punteros_nivel1[indice_nivel1];
                           }
                           else{
                               ptr=punteros_nivel1[indice_nivel1];
                           }
                       }
                   }
               }
               // escribimos en el dispositivo el inodo actualizado
               escribir_inodo(ninodo,inodo);
               break;
       }
       //printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
       return ptr;
   }
   return ptr;
}
