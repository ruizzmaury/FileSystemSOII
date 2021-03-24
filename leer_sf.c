#include "ficheros_basico.h"

int main(int argc, char **argv)
{

    // Check if number of parameters are correct
    if (argc != 2)
    {
        fprintf(stderr, "Sintaxis Error:  ./leer_sf <nombre_dispositivo> \n");
        return EXIT_FAILURE;
    }

    char *nombre = argv[1];
    // mounting virtual device
    if (bmount(nombre) == 1)
    {
        fprintf(stderr, "Mount error in virtual device. \n");
        return EXIT_FAILURE;
    }

    // Lee el superbloque del disco.
    struct superbloque SB;
    if (bread(0, &SB) == -1)
    {
        fprintf(stderr, "Error de lectura del superbloque.\n");
        return EXIT_FAILURE;
    }

    // Print the superblok content.
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);
    printf("\n\n");
    printf("sizeof struct superbloque: %ld\n", sizeof(SB));
    printf("sizeof struct inodo: %d\n", INODOSIZE);

    for (int i = 0; i < ; i++)
    {
        
    }
    

    bumount();
}