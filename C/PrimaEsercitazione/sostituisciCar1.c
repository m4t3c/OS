#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd;                 /* Variabile per la open */
    char Cx;                /* Carattere passato come secondo parametro */
    char Change;            /* Carattere passato come terzo parametro */
    char ch;                /* Singolo carattere letto */
    /* ------------------------------ */

    /* Controllo che siano passati esattamente 3 parametri */
    if (argc != 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di esattamente 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che il primo parametro sia un file apribile in lettura */
    if ((fd = open(argv[1], O_RDWR)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }
    
    /* Controllo che il secondo parametro sia un singolo carattere */
    if ((strlen(argv[2])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[2]);
        exit(3);
    }

    /* Salvo il secondo parametro in Cx */
    Cx=argv[2][0];

    /* Controllo che il terzo parametro sia un singolo carattere */
    if ((strlen(argv[3])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[3]);
        exit(4);
    }

    /* Salvo il terzo parametro in Change */
    Change=argv[3][0];
    
    /* Itero un ciclo che legge tutti i caratteri del file */
    while (read(fd, &ch, 1))
    {
        /* Controllo se il carattere letto è uguale al carattere passato come secondo parametro */
        if (ch == Cx)
        {
            /* Mi sposto indietro di una posizione nel file e sostituisco il carattere */
            lseek(fd, -1L, 1);
            write(fd, &Change, 1);
        }
        
    }
    
    exit(0);
}