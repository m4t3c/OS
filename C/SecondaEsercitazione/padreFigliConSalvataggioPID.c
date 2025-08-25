#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    casuale++;
    return casuale;
}

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int *pid;                           /* Per fork */
    int N;                              /* Numero di figli da creare */
    int n;                              /* Indice per il ciclo for */
    int j;                              /* Indice per recuperare il numero d'ordine di creazione dei figli */
    int pidFiglio, status, ritorno;     /* Per wait */
    /* ------------------------------ */

    /* Controllo che sia passato esattamente un parametro */
    if (argc != 2)
    {
        printf("Erroe nel numero dei parametri: ho bisogno di un parametro ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il parametro passato sia un intero strettamente positivo */
    if ((N = atoi(argv[1])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[1]);
        exit(2);
    }
    
    /* Controllo che N sia strettamente minore di 155 */
    if (N >= 155)
    {
        printf("Errore nel passaggio dei parametri: %d non è strettamente minore di 155\n", N);
        exit(3);
    }
    
    /* Stampo il pid del processo corrente e il numero di processi figli da creare */
    printf("Il processo padre con PID: %d deve creare %d processi figli\n", getpid(), N);
    srand(time(NULL));

    /* Alloco spazio per un array di N interi */
    if ((pid = (int *)malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nell'allocazione dell'array di pid\n");
        exit(4);
    }
    

    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[n] = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(5);
        }
        if (pid[n] == 0)
        {
            /* Processo figlio */
            /* Stampo il pid del processo figlio e il suo indice d'ordine */
            printf("Il processo figlio di indice %d ha PID: %d\n", n, getpid());

            /* Genero il numero random e lo ritorno al padre */
            exit(mia_random(100 + n));
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
            exit(6);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else 
        {
            ritorno = (int)((status >> 8) & 0xFF);
            /* Itero un ciclo che cerca l'indice d'ordine del processo figlio su cui è stata fatta la wait */
            for (j = 0; j < N; j++)
            {
                if (pid[j] == pidFiglio)
                {
                    printf("Il processo figlio di indice %d con PID: %d ha ritornato %d\n", j, pidFiglio, ritorno);
                    break;
                }
                
            }
            
        }
    }
    
    exit(0);
}