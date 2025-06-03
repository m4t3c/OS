#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define MSGSIZE 3

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di parametri passati */
    int Npipe;                      /* Numero di pipe da creare */
    int pid;                        /* Per fork */
    pipe_t *piped;                  /* Array di pipe */
    int n, i;                       /* Indici per i cicli */
    int fd;                         /* File descriptor per la open */
    char linea[MSGSIZE];            /* Array che contiene ogni linea del primo file */
    char buffer[MSGSIZE];           /* Array che contiene i nomi dei vari file dopo il primo */
    int status, ritorno;            /* Per wait */
    /* ------------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, nomefile3, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Assegno ad N il numero di parametri passati e a Npipe il numero di pipe da creare */
    N = argc - 1;
    Npipe = N - 1;

    /* Alloco memoria per Npipe pipe */
    piped = (pipe_t *)malloc(Npipe * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (piped == NULL)
    {
        printf("Errore nella malloc\n");
        exit(2);
    }
    
    /* Creo le Npipe pipe */
    for (i = 0; i < Npipe; i++)
    {
        /* Controllo che la creazione della pipe sia andata a buon fine */
        if (pipe(piped[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }
    
    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork abbia successo */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(4);
        }
        if (pid == 0)
        {
            if (n == 0)
            {
                /* Primo processo figlio */
                /* Si decide di ritornare N + x come valore di errore dove x = 0 e incrementa ad ogni errore */
                /* Chiudo le pipe inutilizzate nel primo processo figlio */
                for (i = 0; i < Npipe; i++)
                {
                    close(piped[i][0]);
                }
            }
            else
            {
                /* Secondo processo figlio */
                /* Si decide di ritornare N + x come valore di errore dove x = 0 e incrementa ad ogni errore */
                /* Chiudo le pipe inutilizzate */
                for (i = 0; i < Npipe; i++)
                {
                    close(piped[i][1]);
                    if (i != n - 1)
                    {
                        close(piped[i][0]);
                    }
                }
            }

            /* Tutti i figli aprono in lettura il file associato */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[n + 1]);
                exit(N);
            }

            /* Se sono nel primo processo figlio leggo tutte le linee e le invio agli altri processi figli */
            if (n == 0)
            {
                while (read(fd, linea, MSGSIZE))
                {
                    /* Termino la stringa e la invio agli altri processi figli */
                    linea[MSGSIZE - 1] = '\0';
                    for (i = 0; i < Npipe; i++)
                    {
                        write(piped[i][1], linea, MSGSIZE);
                    }
                }
            }
            else
            {
                /* Itero un ciclo che va avanti finche' ci sono nomi di file sulla pipe */
                while (read(piped[n - 1][0], buffer, MSGSIZE))
                {
                    /* Itero un ciclo che legge tutti i nomi dei file che ci sono nel file associato al processo figlio */
                    while (read(fd, linea, MSGSIZE))
                    {
                        /* Termino la linea appena letta */
                        linea[MSGSIZE - 1] = '\0';

                        /* Creo il processo nipote */
                        if ((pid = fork()) < 0)
                        {
                            printf("Errore nella creazione del processo nipote di indice n = %d\n", n - 1);
                            exit(N + 1);
                        }
                        if (pid == 0)
                        {
                            /* Processo nipote */
                            /* Si ritorna -1 (255 senza segno) in caso di errore */
                            /* Chiudo la pipe che non sono utilizzate */
                            close(piped[n - 1][0]);

                            /* Ridireziono su /dev/null lo standard output e lo standard error */
                            close(1);
                            open("/dev/null", O_WRONLY);
                            close(2);
                            open("/dev/null", O_WRONLY);

                            /* Eseguo il comando diff */
                            execlp("diff", "diff", buffer, linea, (char *)0);

                            /* Non si dovrebbe mai arrivare qui' */
                            exit(-1);
                        }
                        
                        /* Processo figlio */
                        /* Il figlio aspetta il nipote */
                        if ((pid = wait(&status)) < 0)
                        {
                            printf("Errore nella wait del processo nipote\n");
                            exit(N + 2);
                        }
                        if ((status & 0xFF) != 0)
                        {
                            printf("Il processo nipote con PID: %d e' terminato in modo anomalo\n", pid);
                        }
                        else
                        {
                            ritorno = (int)((status >> 8) & 0xFF);
                            if (ritorno == 0)
                            {
                                printf("I file %s e %s sono uguali\n", buffer, linea);
                            }
                        }
                    }
                    lseek(fd, 0L, 0);
                }
            }

            exit(n);
        }
    }
    
    /* Processo padre */
    /* Chiudo le pipe */
    for (i = 0; i < Npipe; i++)
    {
        close(piped[i][0]);
        close(piped[i][1]);
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
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