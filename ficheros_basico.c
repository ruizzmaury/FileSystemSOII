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