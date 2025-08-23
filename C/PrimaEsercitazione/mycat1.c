#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv){
    
    /* ------ Variabili Locali ------ */
    int buff[BUFSIZ];               /* Buffer per lettura e scrittura */
    int N;                          /* Numero di parametri passati */
    int i;                          /* Indice per scorrere tutti i file da aprire */
    int fd=0;                       /* Variabile per la open, di default su standard input */
    int finito=0;                   /* Variabile per il ciclo di apertura, di default su false*/
    int nread;                      /* numero di caratteri letti dalla read */
    /* ------------------------------- */

    /* Controllo se sono stati passati più di un parametro */
    if (argc > 2)
    {
        N = argc - 1;
    }
    
    /* Inizializzo i a 0 */
    i = 0;

    while (!finito)
    {
        /* Nel caso in cui sia stato passato almeno un parametro apro un file alla volta */
        if (argc >= 2)
        {
            /* Controllo che l'apertura del file vada a buon fine */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[i + 1]);
                exit(1);
            }
            
        }

        /* Incremento di 1 il valore di i */
        i++;

        /* Se è stato passato almeno un parametro stampo i caratteri del file aperto, in caso contrario stampo i caratteri ottenuti dallo standard input e termino il ciclo */
        while ((nread = read(fd, buff, BUFSIZ)) > 0)
        {
            write(1, buff, nread);
        }

        /* Se non è stato passato alcun parametro oppure sono stati letti tutti i file faccio terminare il ciclo */
        if (argc == 1 || i == N)
        {
            finito=1;
        }
        
    }
    
	exit(0);
}