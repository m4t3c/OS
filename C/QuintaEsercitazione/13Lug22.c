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
    int fd;                         /* File descriptor per la open */
    pipe_t *pipes;                  /* Array di pipe */
    int nChar;  	                /* Contatore di occorrenze trovate del carattere cercato nel file */
    char c;                         /* Buffer di caratteri per la read */
    char ok;                        /* Carattere che autorizza il figlio successivo a scrivere */
    int nr, nw;                     /* Per read e write */
    int ritorno;                    /* Per processi figli e wait */
    int pidFiglio, status;          /* Per wait */
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
        if (strlen(argv[j]) > 1)
        {
            printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[j]);
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
        if (pid == 0)
        {
            /* Processo figlio */
            /* Decido di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo le pipe che non mi servono */
            for (j = 0; j < Q + 1; j++)
            {
                if (j != q)
                {
                    close(pipes[j][0]);
                }
                if (j != q + 1)
                {
                    close(pipes[j][1]);
                }
            }
            
            /* Apro in lettura il file passato come primo parametro */
            if((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file %s\n", argv[1]);
                exit(-1);
            }

            /* Imposto a 0 il contatore di caratteri per ogni singola linea */
            nChar = 0;

            /* Itero un ciclo per leggere tutti i caratteri del file */
            while (read(fd, &c, 1) != 0)
            {
                if (c == '\n')
                {
                    /* Aspetto l'ok dal figlio precedente */
                    nr = read(pipes[q][0], &ok, 1);
                    if (nr != 1)
                    {
                        printf("Errore: Figlio %d ha letto un numero di byte sbagliati %d\n", q, nr);
                        exit(-1);
                    }
                    
                    /* Stampo la stringa del testo */
                    printf("Figlio con indice %d e pid %d ha letto %d caratteri %c nella linea corrente\n", q, getpid(), nChar, argv[q + 3][0]);

                    /* Do l'ok al figlio successivo */
                    nw = write(pipes[q + 1][1], &ok, 1);
                    if (nw != 1)
                    {
                        printf("Errore: Figlio %d ha scritto un numero di byte sbagliati %d\n", q, nw);
                        exit(-1);
                    }

                    ritorno = nChar;
                    nChar = 0;
                }
                else
                {
                    if (c == argv[q + 3][0])
                    {
                        nChar++;
                    }
                }
            }
            /* Ogni filgio ritorna le occorrenze dell'ultima linea */
            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe che non servono */
    for (q = 0; q < Q + 1; q++)
    {
        if (q != Q)
        {
            close(pipes[q][0]);
        }

        if (q != 0)
        {
            close(pipes[q][1]);
        }
    }
    
    /* Ciclo che indica che riga del file si sta analizzando */
    for (j = 0; j < L; j++)
    {
        printf("Linea %d del file %s:\n", j, argv[1]);

        /* Annuncio al primo figlio che può scrivere */
        nw = write(pipes[0][1], &ok, 1);
        /* Controllo che il numero di byte scritti sia giusto */
        if (nw != 1)
        {
            printf("Errore: Il processo padre ha scritto un numero di byte sbagliati\n");
        }
        
        /* Ricevo l'ok dall'ultimo processo figlio */
        nr = read(pipes[Q][0], &ok, 1);
        if (nr != 1)
        {
            printf("Errore: Il processo padre ha letto un numero di byte sbagliati\n");
        }
    }
    
    /* Il padre aspetta i figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la wait abbia successo */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio con PID: %d e' terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}