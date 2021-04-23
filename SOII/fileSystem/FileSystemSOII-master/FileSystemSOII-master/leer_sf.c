
#include "ficheros_basico.c"

int showSuperBlockData(struct superbloque SB);
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
    showSuperBlockData(SB);
    struct inodo inodo;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    int indice = reservar_inodo('f', 6);
    traducir_bloque_inodo(indice, 8, 1);
    traducir_bloque_inodo(indice, 204, 1);
    traducir_bloque_inodo(indice, 30004, 1);
    traducir_bloque_inodo(indice, 400004, 1);
    traducir_bloque_inodo(indice, 468750, 1);
    leer_inodo(indice, &inodo);
    printf("Tipo: %c\n", inodo.tipo);
    printf("Permisos: %d\n", inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S \n", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S \n", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S \n", ts);
    printf("ATIME: %s MTIME: %s CTIME: %s\n", atime, mtime, ctime);
    printf("nLinks = %d\n", inodo.nlinks);
    printf("tamEnBytesLog = %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados = %d\n", inodo.numBloquesOcupados);

    bumount();

    return EXIT_SUCCESS;
}

int showSuperBlockData(struct superbloque SB)
{
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
}