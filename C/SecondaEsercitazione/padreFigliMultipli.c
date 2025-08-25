#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                              /* Numero di figli */
    int pid;                            /* Per fork */
    int n;                              /* Indice per i figli */
    int pidFiglio, status, ritorno;     /* Per wait */
    /* ------------------------------ */

    /* Controllo che sia passato esattamente un parametro */
    if (argc != 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di esattamente 1 parametro ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che il parametro passato sia un intero strettamente positivo */
    if ((N = atoi(argv[1])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[1]);
        exit(2);
    }

    /* Stampo il pid del processo podre e il numero di figli da creare */
    printf("Il processo padre con PID: %d creerà %d processi figli\n", getpid(), N);

    /* Itero un ciclo che crei gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice n = %d\n", n);
            exit(3);
        }
        if (pid == 0) 
        {
            /* Processo figlio */
            printf("Sono il processo figlio di indice %d e ho PID: %d\n", n, getpid());

            exit(n);
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
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}