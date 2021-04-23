#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo))
    {
        fprintf(stderr, "Error de lectura \n");
        return EXIT_FAILURE;
    }

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "Error en permisos.\n");
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    char buff[BLOCKSIZE];
    if (bread(bloquef, buff) == -1)
    {
        perror("Error");
        return -1;
    }

    if (primerBL == ultimoBL)
    {
        memcpy(buff + desp1, buf_original, nbytes);
    }
    else
    {
        memcpy(buff + desp1, buf_original, BLOCKSIZE - desp1);
    }
}