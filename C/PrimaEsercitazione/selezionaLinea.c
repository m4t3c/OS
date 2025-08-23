#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd;                 /* Per la open */
    int n;                  /* Variabile che salva il secondo parametro passato */
    char buffer[255];       /* Buffer che conterrà la linea letta */
    int i;                  /* Numero di linee lette */
    int j;                  /* Indice per il carattere letto della linea corrente */
    int trovata = 0;        /* Variabile di controllo per il cilco while, di default a 0 */
    /* ------------------------------ */

    /* Controllo che siano passati esattamente 2 parametri */
    if (argc != 3)
    {
        printf("Errrore nel numero dei parametri: ho bisogno di esattamete 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un file apribile in lettura */
    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }

    /* Controllo che il secondo parametro sia un numero intero strettamete positivo */
    if ((n = atoi(argv[2])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un numero strettamente positivo\n", argv[2]);
        exit(3);
    }
    
    /* Inizializzo i ad 1 e j a 0 */
    i = 1;
    j = 0;

    /* Itero un ciclo che legge un carattere alla volta dal file */
    while ((read(fd, &buffer[j], 1)) != 0)
    {
        /* Controllo se sono arrivato alla fine della linea */
        if (buffer[j] == '\n')
        {
            /* Controllo se la linea corrente è quella cercata */
            if (i == n)
            {
                /* Trasformo la linea in stringa, la stampo, imposto trovata a 1 e esco dal ciclo */
                buffer[j + 1] = 0;
                printf("%s", buffer);
                trovata = 1;
                break;
            }

            /* In caso contrario incremento i e ri inizializzo j a 0 */
            i++;
            j = 0;
        } else {
            j++;
        }
        
    }

    /* Controllo se la linea è stata trovata */
    if (trovata == 1)
    {
        printf("La linea numero %d è stata trovata correttamente\n", n);
    } else
    {
        printf("Non è stata trovata la linea numero %d\n", n);
    }

    exit(0);
}