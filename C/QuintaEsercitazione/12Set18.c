#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di parametri passati */
    int fd;                         /* Per open */
    int pid;                        /* Per fork */
    pipe_t *pipes_pf;               /* Array di pipe tra padre e figlio */
    pipe_t *pipes_pn;               /* Array di pipe tra padre e nipote */
    int i, j;                       /* Indici */
    char c;                         /* Singolo carattere letto */
    long int trasf;                 /* Numero di occorrenze di caratteri modificati */
    int nr;                         /* Variabile per salvare i caratteri letti con la read */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano stati passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel passaggio dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di parametri passati nella variabile N */
    N = argc - 1;

    /* Alloco memoria per gli array di pipe */
    if ((pipes_pf = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe tra padre e figlio\n");
        exit(2);
    }

    if ((pipes_pn = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe tra padre e nipote\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che l'iesima pipe padre-figlio sia creata correttamente */
        if (pipe(pipes_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe padre figlio di indice i = %d\n", i);
            exit(4);
        }

        /* Controllo che l'iesima pipe padre-nipote sia creata correttamente */
        if (pipe(pipes_pn[i]) < 0)
        {
            printf("Errore nella creazione della pipe padre nipote di indice i = %d\n", i);
            exit(5);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che il processo figlio sia creato correttamente */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo filgio */
            /* Chiudo le pipe tra padre e figlio che non servono */
            for (j = 0; j < N; j++)
            {
                close(pipes_pf[j][0]);
                if (j != i)
                {
                    close(pipes_pf[j][1]);
                }
            }

            /* Controllo che il processo nipote sia creato correttamente */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella fork del processo nipote di indice i = %d\n", i);
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Chiudo i lati della pipe che non servono */
                close(pipes_pf[i][1]);
                for (j = 0; j < N; j++)
                {
                    close(pipes_pn[j][0]);
                    if (j != i)
                    {
                        close(pipes_pn[j][1]);
                    }
                }

                /* Apro in lettura il file associato al processo nipote */
                if ((fd = open(argv[i + 1], O_RDWR)) < 0)
                {
                    printf("Errore nella open del file %s\n", argv[i + 1]);
                    exit(-1);
                }

                /* Inizializzo trasf a 0 */
                trasf = 0L;

                /* Itero un ciclo che legge un carattere alla volta il file */
                while (read(fd, &c, 1))
                {
                    /* Controllo che il carattere letto sia un carattere alfabetico minuscolo */
                    if (islower(c))
                    {
                        c = toupper(c);
                        lseek(fd, -1L, SEEK_CUR);
                        write(fd, &c, 1);
                        trasf++;
                    }
                }

                /* Scrivo nella pipe tra padre e nipote il numero di trasformazioni effettuate */
                write(pipes_pn[i][1], &trasf, sizeof(trasf));

                /* Ritorno il codice adeguato in base alle trasformazioni */
                ritorno = trasf / 256;
                exit(ritorno);
            }

            /* Processo figlio */
            /* Chiudo le pipe del nipote */
            for (j = 0; j < N; j++)
            {
                close(pipes_pn[j][0]);
                close(pipes_pn[j][1]);
            }

            /* Apro in lettura il file associato al processo figlio */
            if ((fd = open(argv[i + 1], O_RDWR)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[i + 1]);
                exit(-1);
            }

            /* Inizializzo trasf a 0 */
            trasf = 0L;

            /* Itero un ciclo che legge un carattere alla volta il file */
            while (read(fd, &c, 1))
            {
                /* Controllo che il carattere letto sia un numero */
                if (isdigit(c))
                {
                    /* Sostituisco il numero con uno spazio */
                    lseek(fd, -1L, SEEK_CUR);
                    c = ' ';
                    write(fd, &c, 1);

                    /* Incremento il numero di trasformazioni */
                    trasf++;
                }
            }

            /* Scrivo nella pipe tra padre e figlio il numero di trasformazioni effettuate */
            write(pipes_pf[i][1], &trasf, sizeof(trasf));

            /* Il filgio aspetta il nipote */
            if ((pidFiglio = wait(&status)) < 0)
            {
                printf("Errore nella wait del processo nipote\n");
                exit(-1);
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d terminato in modo anomalo\n", pidFiglio);
            }
            else
            {
                printf("Il processo nipote con PID: %d ha ritornato %d\n", pidFiglio, ritorno = (int)((status >> 8) & 0xFF));
            }

            /* Ritorno il codice adeguato in base alle trasformazioni */
            ritorno = trasf / 256;
            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        close(pipes_pf[i][1]);
        close(pipes_pn[i][1]);
    }

    /* Il padre recupera le informazioni dai figli */
    for (i = 0; i < N; i++)
    {
        /* Leggo prima dalla pipe tra padre e figlio poi tra padre e nipote */
        nr = read(pipes_pf[i][0], &trasf, sizeof(trasf));
        if (nr != sizeof(trasf))
        {
            printf("Errore nella read dalla pipe tra padre e figlio di indice i = %d\n", i);
            exit(7);
        }
        printf("Nel file %s sono state trovate %ld occorrenze di caratteri numerici\n", argv[i + 1], trasf);

        nr = read(pipes_pn[i][0], &trasf, sizeof(trasf));
        if (nr != sizeof(trasf))
        {
            printf("Errore nella read dalla pipe tra padre e nipote di indice i = %d\n", i);
            exit(8);
        }
        printf("Nel file %s sono state trovate %ld occorrenze di caratteri alfabetici minuscoli\n", argv[i + 1], trasf);
    }

    /* Il padre aspetta i figli*/
    for (i = 0; i < N; i++)
    {
        /* Contorllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait del processo figlio\n");
            exit(9);
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