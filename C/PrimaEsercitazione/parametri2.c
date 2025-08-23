#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;              /* Secondo parametro passato */
    char C;             /* Terzo parametro passato */
    /* ------------------------------- */

    /* Controllo che siano passati esattamente 3 parametri */
    if (argc != 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di 3 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un file */
    if (open(argv[1], O_RDONLY) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file\n", argv[1]);
        exit(2);
    }

    /* Controllo che il secondo parametro sia un numero intero */
    if ((N = atoi(argv[2])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un numero intero\n", argv[2]);
        exit(3);
    }
    
    /* Controllo che il terzo parametro sia un singolo carattere */
    if (strlen(argv[3]) != 1)
    {
        printf("Erorre nel passaggio dei parametri: %s non è un singolo carattere\n", argv[3]);
        exit(4);
    }

    C = argv[3][0];
    
    /* Stampo il nome dell'eseguibile e il numero di dati passati */
    printf("A %s sono stati passati correttamente 3 parametri\n", argv[0]);

    /* Stampo i valori dei vari parametri passati */
    printf("Il primo parametro passato corrisponde al file %s\n", argv[1]);
    printf("Il secondo parametro passato corrisponde al numero intero %d\n", N);
    printf("Il terzo parametro passato corrisponde al carattere %c\n", C);

    exit(0);
}