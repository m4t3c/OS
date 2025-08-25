#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PERM 0644

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di parametri passati */
    int n;                          /* Indice per i processi figli */
    int fdw;                        /* Per creat */
    int pid;                        /* Per fork */
    char *FOut;                  /* Stringa con il nome del file + .sort */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di parametri passati */
    N = argc - 1;

    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione del processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo nipote di indice n = %d\n", n);
            exit(2);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Alloco memoria per la stringa del nome del file da creare */
            FOut = (char *)malloc(strlen(argv[n + 1] + 6));
            if (FOut == NULL)
            {
                printf("Errore nella malloc\n");
                exit(-1);
            }
            
            /* Creo la stringa */
            strcpy(FOut, argv[n + 1]);
            strcat(FOut, ".sort");

            /* Cotrnollo che la creazione del file vada a buon fine */
            if ((fdw = creat(FOut, PERM)) < 0)
            {
                printf("Errore nella creazione del file %s\n", FOut);
                exit(-1);
            }

            /* Chiudo il file perchè il processo figlio non lo usa */
            close(fdw);

            /* Controllo che la creazione del processo nipote vada a buon fine */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote\n");
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Chiudo lo standard input per redirezionarlo sul file argv[n + 1] */
                close(0);
                if (open(argv[n + 1], O_RDONLY) < 0)
                {
                    printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[n + 1]);
                    exit(-1);
                }
                
                /* Chiudo lo standard output per redirezionarlo sul file FOut */
                close(1);
                if (open(FOut, O_WRONLY) < 0)
                {
                    printf("Errore nell'apertura del file %s\n", FOut);
                    exit(-1);
                }
                
                /* Eseguo il comando sort */
                execlp("sort", "sort", (char *)0);

                /* Non si dovrebbe arrivare quì */
                printf("Errore nell'esecuzione del comando sort\n");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Il figlio aspetta il nipote */
            if ((pidFiglio = wait(&status)) < 0)
            {
                printf("Errore nella wait del nipote\n");
                exit(-1);
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d terminato in modo anomalo\n", pidFiglio);
            }
            
            ritorno = (int)((status >> 8) & 0xFF);
            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait del processo figlio\n");
            exit(3);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}