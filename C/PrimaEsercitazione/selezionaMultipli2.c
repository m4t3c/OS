#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd;             /* Variabile per la open */
    int n;              /* Secondo parametro passato*/
    int i = 1;          /* Numero del multiplo di n */
    long int pos;       /* Posizione del carattere da leggere */
    long int len;       /* Lunghezza del file */
    char ch;            /* Singolo carattere letto */
    /* ------------------------------ */

    /* Controllo che siano passati esattamente 2 parametri */
    if (argc != 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un file e che sia apribile in lettura */
    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }
    
    /* Controllo che il secondo parametro sia un numero strettamente positivo */
    if ((n = atoi(argv[2])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un numero intero strettamente positivo\n", argv[2]);
        exit(3);
    }

    /* Calcolo la lunghezza del file con lseek */
    len = lseek(fd, 0, SEEK_END);

    /* Inizializzo pos a 0 */
    pos = 0L;

    /* Itero un ciclo che legge tutti i caratteri del file */
    while (pos < len)
    {
        /* Calcolo la posizione del carattere che deve essere letto */
        pos = (long int)(i * n);

        /* Contorllo che la posizione del carattere da leggere sia ancora compresa nel file */
        if (pos < len)
        {
            /* Sposto il puntatore del file alla posizione desiderata e leggo il carattere */
            lseek(fd, pos - 1, SEEK_SET);
            read(fd, &ch, 1);

            /* Stampo su standard output il carattere letto */
            printf("Il carattere multiplo di %d numero %d all'interno del file %s è %c\n", n, i, argv[1], ch);

            /* Incremento i di 1 */
            i++;
        }
        
    }
    
    exit(0);
}