#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili Locali ------ */
    int fd;                         /* Per open */
    int L;                          /* Lunghezza in linee del file */
    int Q;                          /* Numero di processi figli */
    int pid;                        /* Per fork */
    pipe_t *pipes;                  /* Array di Q + 1 pipe */
    int q, i;                       /* Indice per i cicli */
    char ok;                        /* Segnale per far eseguire la lettura e la scrittura al processo successivo */
    char ch;                        /* Singolo carattere letto */
    int nrChar;                     /* Occorrenze del carattere cercato in ogni linea */
    int nr, nw;                     /* Numero di caratteri letti e scritti su pipe */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 4 parametri */
    if (argc < 5)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 4 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il secondo parametro sia un numero intero strettamente positivo */
    if ((L = atoi(argv[2])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[2]);
        exit(2);
    }

    /* Salvo in Q il numero di processi figli da creare */
    Q = argc - 3;

    /* Controllo che i successivi parametri siano singoli caratteri */
    for (q = 0; q < Q; q++)
    {
        if (strlen(argv[q + 3]) != 1)
        {
            printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[q + 3]);
            exit(3);
        }
    }

    /* il padre imposta di IGNORARE il segnale SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Alloco memoria per le Q + 1 pipe */
    if ((pipes = (pipe_t *)malloc((Q + 1) * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(4);
    }

    /* Creo le pipe di comunicazione tra padre e figli */
    for (q = 0; q <= Q; q++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(pipes[q]) < 0)
        {
            printf("Errore nella creazione della pipe di indice q = %d\n", q);
            exit(5);
        }
    }

    /* Creo i Q processi figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nela fork\n");
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe che non servono */
            for (i = 0; i <= Q; i++)
            {
                if (i != q)
                {
                    close(pipes[i][0]);
                }
                if (i != q + 1)
                {
                    close(pipes[i][1]);
                }
            }

            /* Apro in lettura il file passato come primo parametro */
            if ((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[1]);
                exit(-1);
            }

            /* Inizializzo il contatore di occorrenze a 0 */
            nrChar = 0;

            /* Itero un ciclo che legge un carattere alla volta il file */
            while (read(fd, &ch, 1))
            {

                /* Controllo se sono arrivato alla fine della riga */
                if (ch == '\n')
                {
                    /* Controllo se ho l'ok dal figlio precedente per scrivere su pipe */
                    nr = read(pipes[q][0], &ok, 1);
                    if (nr != sizeof(ok))
                    {
                        printf("Errore: Figlio %d ha letto un numero errato di byte %d\n", q, nr);
                        exit(-1);
                    }

                    /* Il figlio stampa quante occorrenze del carattere cercato ha trovato nella linea corrente */
                    printf("Figlio con indice %d e PID: %d ha letto %d caratteri %s nella linea corrente\n", q, getpid(), nrChar, argv[q + 3]);

                    /* Do l'ok al processo successivo */
                    nw = write(pipes[q + 1][1], &ok, 1);
                    if (nw != sizeof(ok))
                    {
                        printf("Errore: Figlio %d ha scritto un numero errato di byte %d\n", q, nw);
                    }
                    
                    ritorno = nrChar;
                    nrChar = 0;
                }
                else
                {
                    /* Controllo se il carattere letto è quello cercato */
                    if (ch == argv[q + 3][0])
                    {
                        /* Incremento il numero di occorrenze */
                        nrChar++;
                    }
                }
            }

            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (q = 0; q <= Q; q++)
    {
        if (q != 0)
        {
            close(pipes[q][1]);
        }
        if (q != Q)
        {
            close(pipes[q][0]);
        }
        
    }
    
    /* Il padre deve dire il numero di linea che si sta analizzando e dare al primo figlio l'ok per scrivere */
    for (i = 0; i < L; i++)
    {
        /* Riporto il numero di riga del file */
        printf("Linea %d del file %s\n", i, argv[1]);
        nw = write(pipes[0][1], &ok, 1);
        if (nw != sizeof(ok))
        {
            printf("Errore: Padre ha scritto un numero errato di byte %d\n", nw);
        }
        
        nr = read(pipes[Q][0], &ok, 1);
        if (nr != sizeof(ok))
        {
            printf("Errore: Padre ha letto un numero errato di byte %d\n", nr);
        }
        
    }
    
    /* Il padre aspetta i figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo filgio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}