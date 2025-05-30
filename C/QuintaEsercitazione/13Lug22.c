#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

/* Definisco il tipo pipe_t come array di due int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------*/
    int L;                          /* Numero di righe del file F */
    int Q;                          /* Numero di processi figli */
    int pid;                        /* Per fork */
    int q, j;                       /* Indici per i cicli */
    pipe_t *pipes;                  /* Array di pipe */

    /* Controllo che siano passati almeno 4 parametri */
    if (argc < 5)
    {
        printf("Errore nel numero di parametri: ho bisogno di almeno 4 parametri (nomefile, nrighefile, car1, car2 ... ) ma argc = %d", argc);
        exit(1);
    }

    /* Ricavo il numero di righe del file F */
    L = atoi(argv[2]);
    /* Controllo che sia un numero strettamente positivo */
    if (L <= 0)
    {
        printf("Errore nel passaggio dei parametri: %d non e' strettamente positivo\n", L);
        exit(2);
    }

    /* Controllo che gli ultimi Q parametri siano singoli caratteri */
    for (j = 3; j < argc; j++)
    {
        if (streln(argv[j]) > 1)
        {
            printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n");
            exit(3);
        }
    }
    
    Q = argc - 3;

    /* Alloco spazio per l'array di pipe */
    pipes = (pipe_t *)malloc((Q + 1) * sizeof(pipe_t));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(4);
    }

    /* Creo le Q + 1 pipe */
    for (q = 0; q < Q + 1; q++)
    {
        if (pipe(pipes[q]) < 0)
        {
            printf("Errore nella creazione della pipe di indice q = %d\n", q);
            exit(5);
        }
    }
    
    /* Ciclo di creazione dei figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la creazione del processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice q = %d\n", q);
            exit(6);
        }
        
    }
    
}