#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 pipe */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int M;                          /* Numero di parametri passati */
    int pid;                        /* Per open */
    pipe_t *pipes;                  /* Array di pipe di comunicazione */
    int lunghezza;                  /* Valore ritornato da ogni figlio */
    int j, k;                       /* Indici */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Inizializzo M con il numero di parametri passati */
    M = argc - 1;

    /* Alloco memoria per l'array di pipe */
    if ((pipes = (pipe_t *)malloc(M * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(2);
    }

    /* Itero un ciclo che crea le M pipe */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(pipes[j]) < 0)
        {
            printf("Errore nella creazione della pipe di indice j = %d\n", j);
            exit(3);
        }
    }

    /* Creo gli M processi figli */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (k = 0; k < M; k++)
            {
                close(pipes[k][0]);
                if (k != j)
                {
                    close(pipes[k][1]);
                }
            }

            /* Inizializzo lunghezza con 3000 + j */
            lunghezza = 3000 + j;

            /* Comunico al padre lunghezza */
            write(pipes[j][1], &lunghezza, sizeof(lunghezza));

            /* Esco con 0 */
            exit(0);
        }
    }

    /* Processo padre */
    /* Chiudo tutte le pipes in scrittura */
    for (j = 0; j < M; j++)
    {
        close(pipes[j][1]);
    }

    /* Il padre recupera le informazioni dai figli */
    for (j = 0; j < M; j++)
    {
        read(pipes[j][0], &lunghezza, sizeof(lunghezza));
        printf("Il processo figlio di indice %d ha comunicato il valore %d per il file %s\n", j, lunghezza, argv[j + 1]);
    }
    
    /* Il padre aspetta i figli */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d ritornato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}