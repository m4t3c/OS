#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd;                 /* Per la open */
    int n;                  /* Secondo parametro passato */
    int i;                  /* Numero di linee trovate che sono lunghe n caratteri */
    int j;                  /* Indice per la read */
    char buffer[255];       /* Buffer per la read */
    /* ------------------------------ */

    /* Controllo che siano passati esattamente 2 parametri */
    if (argc != 3)
    {
        printf("Errore nel numero di parametri: ho bisogno di esattamente 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un file apribile in lettura */
    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }

    /* Controllo che il secondo parametro sia un intero strettamente positivo */
    if ((n = atoi(argv[2])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[2]);
        exit(3);
    }
    
    /* Inizializzo i e j a 0 */
    i = 0;
    j = 0;
    
    /* Itero un ciclo che termina una volta terminata la lettura di tutto il file */
    while ((read(fd, &buffer[j], 1)) != 0)
    {
        /* Controllo se sono arrivato alla fine della linea corrente */
        if (buffer[j] == '\n')
        {
            /* Controllo se la linea ha n - 1 caratteri */
            if (j == n - 1)
            {
                /* Aggiungo il terminatore alla linea letta */
                buffer[j + 1] = 0;
                
                /* Incremento il numero di linee trovate e stampo su standard output */
                i++;
                printf("Linea trovata numero %d: %s", i, buffer);
            }
            j = 0;
        } else {
            j++;
        }
    
    }
    
    /* Se i = 0 vuol dire che non sono state trovate linee lunghe n - 1 caratteri */
    if (i == 0)
    {
        printf("Nessuna linea nel file %s è lungha %d caratteri (compreso il terminatore)\n", argv[1], n);
    }
    
    exit(0);
}