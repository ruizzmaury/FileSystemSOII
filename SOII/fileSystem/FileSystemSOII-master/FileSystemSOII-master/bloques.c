#include "bloques.h"

static int descriptor = 0;

int bmount(const char *camino)
{
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    return descriptor;
}

int bumount()
{
    if (close(descriptor) != -1)
    {
        return EXIT_SUCCESS;
    }

    // sino devuelve el error
    return EXIT_FAILURE;
}

// write 1 block from de virtual device
int bwrite(unsigned int nbloque, const void *buf)
{
    size_t nbytes;
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) != -1)
    {
        nbytes = write(descriptor, buf, BLOCKSIZE);
        if (nbytes < 0)
        {
            return EXIT_FAILURE;
        }
        else
        {
            return BLOCKSIZE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }
}

// read 1 block from de virtual device
int bread(unsigned int nbloque, void *buf){
    size_t nbytes;
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) != -1)
    {
        nbytes = read(descriptor, buf, BLOCKSIZE);
        if (nbytes < 0)
        {
            return EXIT_FAILURE;
        }
        else
        {
            return BLOCKSIZE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }
}