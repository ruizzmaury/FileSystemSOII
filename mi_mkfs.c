//#include "bloques.c"
#include "ficheros_basico.c"

int main(int argc, char **argv)
{
    // Check if number of parameters are correct
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis Error:  ./mi_mkfs <nombre_dispositivo> <nbloques> \n");
        return EXIT_FAILURE;
    }

    // assigning received values
    char *nombre = argv[1];
    int nbloques = atoi(argv[2]);

    // creating buffer
    unsigned char buff[BLOCKSIZE];
    memset(buff, 0, BLOCKSIZE);

    // mounting virtual device
    if (bmount(nombre) == 1)
    {
        fprintf(stderr, "Mount error in virtual device. \n");
        return EXIT_FAILURE;
    }

    // initializing the file used as virtual device
    for (int i = 0; i < nbloques; i++)
    {
        if (bwrite(i, buff) == 1)
        {
            fprintf(stderr, "Write error on the device. \n");
            return EXIT_FAILURE;
        }
    }
    // ------ metadata initialization --------------------

    int ninodos = nbloques / 4; // max number of files and directories
                                // possible in the system

    

    if (initSB(nbloques, ninodos) == 1)
    {
        fprintf(stderr, "Error initialization superblock data. \n");
        return EXIT_FAILURE;
    }

    initMB();

    if (initAI() == 1)
    {
        fprintf(stderr, "Error initialization free inodos list. \n");
        return EXIT_FAILURE;
    }

//----------------------------------------------------

    // dismount virtual device
    bumount();
}