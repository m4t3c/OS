#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                 /* Numero di file passati come parametro */
    char Cx;               /* Carattere passato come ultimo parametro */
    int fd;                /* Per open */
    int pid;               /* Per fork */
    long int pos;          /* Posizione da comunicare al padre */
    pipe_t *pipes_pf;      /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t *pipes_fp;      /* Array di pipe di comunicazione tra figlio e padre */
    int n, i;              /* Indici per i cicli */
    int ritorno = 0;       /* Valore ritornato dal processo figlio (numero di caratteri trasformati) */
    char c;                /* Singolo carattere letto */
    char cx;               /* Carattere letto da standard input */
    char scarto;           /* Variabile usata per eliminare il carattere '\n' letto da standard input */
    int nr;                /* Variabile che serve al padre per sapere se ha letto qualcosa */
    int finito;            /* Variabile di controllo */
    int pidFiglio, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di file passati come parametro in N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore: %s non è un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }

    /* Salvo in Cx l'ultimo parametro */
    Cx = argv[argc - 1][0];

    /* Alloco memoria per gli array di pipe */
    pipes_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipes_fp = (pipe_t *)malloc(N * sizeof(pipe_t));

    /* Controllo che le malloc siano andate a buon fine */
    if ((pipes_pf == NULL) || (pipes_fp == NULL))
    {
        printf("Errore nella malloc\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        if (pipe(pipes_pf[n]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra padre e figlio\n");
            exit(4);
        }

        if (pipe(pipes_fp[n]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra figlio e padre\n");
            exit(5);
        }
    }

    /* Genero gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (i = 0; i < N; i++)
            {
                close(pipes_fp[i][0]);
                close(pipes_pf[i][1]);
                if (i != n)
                {
                    close(pipes_pf[i][0]);
                    close(pipes_fp[i][1]);
                }
            }

            /* Apro il file associato al processo figlio in lettura e scrittura */
            if ((fd = open(argv[n + 1], O_RDWR)) < 0)
            {
                printf("Errore nella open di %s\n", argv[n + 1]);
                exit(-1);
            }

            /* Itero un ciclo che legge un carattere alla volta il file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == Cx)
                {
                    /* Torno indietro di una posizione con l'lseek e assegno a pos il valore della posizione del carattere */
                    pos = lseek(fd, 0L, SEEK_CUR);

                    /* Invio al padre la posizione */
                    write(pipes_fp[n][1], &pos, sizeof(pos));

                    /* Leggo il carattere inviato dal padre */
                    read(pipes_pf[n][0], &cx, 1);

                    /* Controllo che il carattere letto sia diverso da '\n' */
                    if (cx != '\n')
                    {
                        /* Sostituisco nel file il carattere che è stato inviato dal processo padre */
                        lseek(fd, -1L, SEEK_SET);
                        write(fd, &cx, 1);
                        ritorno++;
                    }
                }
            }

            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (n = 0; n < N; n++)
    {
        close(pipes_pf[n][0]);
        close(pipes_fp[n][1]);
        
    }
    
    /* Inizializzo finito a 0 */
    finito = 0;

    /* Itero un ciclo che finisce quando finito sarà a 1 */
    while (!finito)
    {
        finito = 1;
        for (n = 0; n < N; n++)
        {
            nr = read(pipes_fp[n][0], &pos, sizeof(pos));
            if (nr != 0)
            {
                finito = 0;
                printf("Il figlio con indice %d ha trovato nel file %s un'occorrenza del carattere %c in posizione %ld\n", n, argv[n + 1], Cx, pos);
                printf("Inserisci il carattere con cui sostituire l'occorrenza del carattere (non scrivere niente per non sostituire):\n");
                read(0, &cx, 1);
                if (cx != '\n')
                {
                    read(0, &scarto, 1);
                }
                
                /* Invio al processo figlio il carattere con cui eventualmente sostituire l'occorrenza di Cx */
                write(pipes_pf[n][1], &cx, 1);
            }
            
        }
        
    }

    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d è terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}