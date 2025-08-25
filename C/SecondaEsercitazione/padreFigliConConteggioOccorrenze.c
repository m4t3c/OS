#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main (int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                              /* Numero di file passati come parametro */
    char Cx;                            /* Carattere passato come ultimo parametro */
    int fd;                             /* Per la open */
    int pid;                            /* Per fork */
    int n;                              /* Indice del figlio corrente */
    char ch;                            /* Carattere letto dalla read */
    int occ = 0;                        /* Numero di occorrenze del carattere Cx nel file n-esimo */
    int pidFiglio, status, ritorno;     /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di file nella variabile N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if ((strlen(argv[argc - 1])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }

    /* Salvo l'ultimo parametro nella variabile Cx */
    Cx = argv[argc - 1][0];

    /* Creo N processi figli con un ciclo for */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(3);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Controllo che l'apertura del file in lettura vada a buon fine */
            if ((fd = open(argv[n + 1], O_RDONLY)) <= 0)
            {
                printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Itero un ciclo che legge un carattere alla volta del file */
            while (read(fd, &ch, 1))
            {
                /* Controllo se il carattere letto è uguale a quello cercato */
                if (ch == Cx)
                {
                    /* Incremento il numero di occorrenze nel file corrente */
                    occ++;
                }
                
            }
            
            /* Restituisco al padre il numero di occorrenze */
            exit(occ);
        }
        
    }
    
    /* Processo padre */
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(4);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d è terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}