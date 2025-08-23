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
    int nread;          /* Numero di caratteri letti */
    char *buff;         /* Buffer degli n caratteri letti*/
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

    /* Alloco dinamicamente l'array buff con n char */
    buff = malloc(n);

    /* Itero un ciclo che legge tutti i caratteri del file */
    while ((nread = read(fd, buff, n)) == n)
    {
        /* Stampo l'n-esimo carattere letto */
        printf("Il carattere multiplo di %d numero %d all'interno del file %s è %c\n", n, i, argv[1], buff[n - 1]);
        /* Incremento i di 1 */
        i++;   
    }
    
    exit(0);
}