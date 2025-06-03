#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    
    /* ------ Variabili locali ------ */
    int N;  	                    /* Numero passato come primo parametro */
    int outfile;                    /* File descriptor per il file creato */
    int pid;                        /* Per fork */
    pipe_t *pipe_pf;                /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t pipe_fn;                 /* Pipe di comunicazione tra figlio e nipote */
    int n, i;                       /* Indici per i cicli */
    char buffer[250];               /* Per la sprintf */
    int status, ritorno;            /* Per wait */
    /* Controllo che siano passati 2 parametri */
    if (argc != 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
        
    /* Controllo che il primo parametro sia un numero strettamente positivo */
    if ((N = atoi(argv[1])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un numero intero strettamente positivo\n", argv[1]);
        exit(2);
    }
     
    /* Controllo che la creazione del file vada a buon fine */
    if ((outfile = creat(argv[2], PERM)) < 0)
    {
        printf("Errore nella creazione del file %s\n", argv[2]);
        exit(3);
    }
    
    /* Alloco memoria per l'array di pipe padre-figlio */
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (pipe_pf == NULL)
    {
        printf("Errore nella malloc\n");
        exit(4);
    }
    
    /* Creo le pipe */
    for (n = 0; n < N; n++)
    {
        if (pipe(pipe_pf[n]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra padre e figlio di indice n = %d\n", n);
            exit(5);
        }
    }
    
    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione del processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo tutte le pipe non necessarie */
            for (i = 0; i < N; i++)
            {
                close(pipe_pf[i][0]);
                if (i != n)
                {
                    close(pipe_pf[i][1]);
                }
            }
            
            /* Creo la pipe tra figlio e nipote */
            if (pipe(pipe_fn) < 0)
            {
                printf("Errore nella creazione della pipe tra processo figlio e nipote\n");
                exit(-1);
            }
            
            /* Creo il processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote di indice n = %d\n", n);
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
                /* Chiudo la pipe del processo figlio rimasta aperta */
                close(pipe_pf[n][1]);

                /* Simulo il piping dei comandi */
                close(1);
                dup(pipe_fn[1]);

                /* Chiudo tutte le pipe del processo nipote */
                close(pipe_fn[0]);
                close(pipe_fn[1]);

                /* Eseguo il comando ps */
                execlp("ps", "ps", (char *)0);

                /* Non si dovrebbe mai arrivare qui'*/
                perror("Errore nella esecuzione del comando ps\n");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Salvo in buffer il PID del nipote */
            sprintf(buffer, "%d", pid);

            /* Simulo il piping dei comandi */
            close(0);
            dup(pipe_fn[0]);
            close(1);
            dup(pipe_pf[n][1]);

            /* Chiudo tutte le pipe */
            close(pipe_fn[0]);
            close(pipe_fn[1]);
            close(pipe_pf[n][1]);

            /* Eseguo il comando grep */
            execlp("grep", "grep", buffer, (char *)0);

            /* Non si dovrebbe mai arrivare qui' */
            perror("Errore nell'esecuzione del comando grep\n");
            exit(-1);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe in scrittura */
    for (n = 0; n < N; n++)
    {
        close(pipe_pf[n][1]);
    }
    
    /* Il padre recupera le informazioni dai figli */
    for (n = 0; n < N; n++)
    {
        i = 0;
        while (read(pipe_pf[n][0], &(buffer[i]), 1))
        {
            i++;
        }
        
        write(outfile, buffer, i);
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            if (ritorno != 0)
            {
                printf("Il figlio con PID: %d ha fallito l'esecuzione del comando grep oppure se 255 ha avuto dei problemi\n", pid);
            }
            else
            {
                printf("Il figlio con PID: %d e' terminato con valore %d\n", pid, ritorno);
            }
        }
    }
    
    exit(0);
}