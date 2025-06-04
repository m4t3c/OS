#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Definisco il tipo tipoL come array di 250 caratteri */
typedef char tipoL[250];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;             /* Numero di parametri/processi figli */
    int fcreato;       /* File descriptor per il file creato */
    int fd;            /* File descriptor per la open */
    int pid;           /* Per fork e wait */
    pipe_t *pipes;     /* Array di pipe */
    int n, i, j, k;    /* Indici per i cicli */
    tipoL *tutteLinee; /* Array di linee */
    tipoL linea;       /* Buffer per la read */
    int nr, nw;        /* Per read e write */
    int ritorno;       /* Per processi figli e wait */
    int status;        /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di parametri nella variabile N */
    N = argc - 1;

    /* Creo il file fcreato */
    if ((fcreato = creat("Coppa", PERM)) < 0)
    {
        printf("Errore nella creazione del file %s\n", "Coppa");
        exit(2);
    }

    /* Alloco memoria per l'array di pipe */
    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione dell'n-esima pipe vada a buon fine */
        if (pipe(pipes[n]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", n);
            exit(4);
        }
    }

    /* Alloco memoria per l'array di linee */
    tutteLinee = (tipoL *)malloc(N * sizeof(tipoL));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (tutteLinee == NULL)
    {
        printf("Errore nella malloc dell'array di linee\n");
        exit(5);
    }

    /* Creo gli N processi filgi */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo le pipe che non servono */
            for (i = 0; i < N; i++)
            {
                if (i != n)
                {
                    close(pipes[i][0]);
                }
                if (i != ((n + 1) % N))
                {
                    close(pipes[i][1]);
                }
            }

            /* Apro il file in lettura */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[n + 1]);
                exit(-1);
            }

            /* Inizializzo a 0 i che fungera' da indice per la read */
            i = 0;

            while (read(fd, &(linea[i]), 1))
            {
                /* Controllo se sono arrivato alla fine di una linea */
                if (linea[i] == '\n')
                {
                    /* Aspetto l'ok dal processo precedente */
                    nr = read(pipes[n][0], tutteLinee, N * sizeof(tipoL));

                    if (nr != N * sizeof(tipoL))
                    {
                        printf("Il processo figlio n = %d ha letto un numero errato di byte %d\n", n, nr);
                        exit(-1);
                    }

                    /* Inserisco la linea nel posto giusto dell'array */
                    memcpy(tutteLinee[n], linea, i + 1);

                    /* Scrivo sulla pipe del processo successivo l'array di linee */
                    nw = write(pipes[(n + 1) % N][1], tutteLinee, N * sizeof(tipoL));
                    /* Controllo che la write sia andata a buon fine */
                    if (nw != N * sizeof(tipoL))
                    {
                        printf("Il processo figlio n = %d ha scritto un numero errato di byte %d\n", n, nw);
                        exit(-1);
                    }

                    /* Controllo se sono nell'ultimo processo figlio */
                    if (n == N - 1)
                    {
                        /* Scrivo sul file creato tutte le linee lette */
                        for (j = 0; j < N; j++)
                        {
                            /* Itero un ciclo per scrivere tutti i caratteri della linea */
                            for (k = 0; k < 250; k++)
                            {
                                write(fcreato, &(tutteLinee[j][k]), 1);
                                /* Se arrivo al carattere '\n' interrompo il ciclo */
                                if (tutteLinee[j][k] == '\n')
                                {
                                    break;
                                }
                            }
                        }
                    }

                    /* Assegno a ritorno il valore di i e poi setto i a 0 */
                    ritorno = i + 1;
                    i = 0;
                }
                else
                {
                    i++;
                }
            }

            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo i lati delle pipe che non servono */
    /* Lascio aperta la prima pipe in lettura e scrittura */
    for (n = 1; n < N; n++)
    {
        close(pipes[n][0]);
        close(pipes[n][1]);
    }

    /* Faccio la prima scrittura su pipe per il primo processo figlio */
    nw = write(pipes[0][1], tutteLinee, N * sizeof(tipoL));
    /* Controllo che la scrittura sia andata a buon fine */
    if (nw != N * sizeof(tipoL))
    {
        printf("Processo padre ha scritto un numero errato di bytes %d\n", nw);
        exit(7);
    }
    
    /* Chiudo la prima pipe in scrittura */
    close(pipes[0][1]);

    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait sia andata a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}
