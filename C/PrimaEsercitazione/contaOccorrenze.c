#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int f;                  /* Variabili per aprire il file e per contare i caratteri letti */
    char Cx;                /* Carattere passato come secondo parametro */
    long int totale = 0;    /* Numero di occorrenze del carattere Cx nel file F */
    char buff[1];           /* Array di 1 char per salvare il carattere letto */
    /* ------------------------------- */

    /* Controllo che siano passati esattamente 2 parametri */
    if (argc != 3) 
    {
        printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un file apribile in lettura */
    if ((f = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }
    
    /* Controllo che il secondo parametro sia un singolo carattere */
    if (strlen(argv[2]) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[2]);
        exit(3);
    }

    /* Salvo il secondo parametro nella variabile Cx */
    Cx = argv[2][0];

    /* Itero un ciclo che legge tutti i caratteri del file */
    while (read(f, buff, 1) == 1)
    {
        /* Controllo se il carattere letto è uguale a Cx */
        if (buff[0] == Cx)
        {
            /* Incremento il numero di occorrenze trovate */
            totale++;
        }
        
    }

    /* Stampo il numero di occorrenze trovate */
    printf("Nel file %s sono state trovate %ld occorrenze del carattere %c\n", argv[1], totale, Cx);

    exit(0);   
}