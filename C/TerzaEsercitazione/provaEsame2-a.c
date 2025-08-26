#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct
{
    char c;     /* Carattere controllato */
    long int n; /* Numero di occorrenze trovate del carattere */
} tipoS;

int main(int argc, char **argv)
{

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di processi figli da creare */
    int fd;                         /* Per la open */
    int pid;                        /* Per fork */
    int n;                          /* Indice dei processi figli */
    int p[2];                       /* Array di 2 interi per la pipe */
    char c;                         /* Carattere letto */
    tipoS occ;                      /* Struttura per la comunicazione tra padre e figli */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di caratteri passati in N */
    N = argc - 2;

    /* Controllo che gli N caratteri siano veramente singoli caratteri */
    for (n = 0; n < N; n++)
    {
        if (strlen(argv[n + 2]) != 1)
        {
            printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[n + 2]);
            exit(3);
        }
    }

    /* Creo la pipe di comunicazione tra padre e figlio */
    if (pipe(p) < 0)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }

    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice n = %d\n", n);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo la pipe in lettura */
            close(p[0]);

            /* Controllo che il primo parametro sia un file apribile in lettura */
            if ((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nella open di %s (file non esiste o non apribile in lettura)\n", argv[1]);
                exit(-1);
            }

            /* Inizializzo la struct */
            occ.c = argv[n + 2][0];
            occ.n = 0L;

            /* Leggo un carattere alla volta il file */
            while ((read(fd, &c, 1)) > 0)
            {
                /* Controllo se il carattere letto è uguale a quello cercato */
                if (c == occ.c)
                {
                    /* Incremento nella struct il numero di occorrenze */
                    occ.n++;
                }
            }

            /* Invio al padre la struct */
            write(p[1], &occ, sizeof(occ));
            exit(occ.c);
        }
    }

    /* Processo padre */
    /* Chiudo la pipe in scrittura */
    close(p[1]);

    /* Recupero le informazioni dai processi figli */
    while (read(p[0], &occ, sizeof(occ)))
    {
        /* Stampo su standard output il carattere e le occorrenze trovate di esso */
        printf("Occorrenze trovate del carattere %c nel file %s: %ld\n", occ.c, argv[1], occ.n);
    }

    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d è terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %c (in decimale %d se 255 problemi!)\n", pidFiglio, ritorno, ritorno);
        }
    }

    exit(0);
}