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
    int N;                          /* Numero di file passati come parametro * 2 */
    int Cz;                         /* Carattere passato come ultimo parametro */
    int pid;                        /* Per fork */
    int fd;                         /* Per open */
    pipe_t *pipes_12;               /* Array di pipe di comunicazione tra primo e secondo figlio */
    pipe_t *pipes_21;               /* Array di pipe di comunicazione tra secondo e primo filgio */
    char c;                         /* Singolo carattere letto */
    int occ;                        /* Numero di occorrenze per ogni figlio di caratteri trovati */
    long int pos;                   /* Posizione corrente del carattere trovato */
    long int posLetta;              /* Posizione letta dalla read */
    int n, i;                       /* Indici per i cicli */
    int nr, nw;                     /* Controlli per read e write */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Inizializzo N come argc - 2 */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore: %s non è un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }

    /* Inizializzo Cx con il carattere passato come ultimo parametro */
    Cz = argv[argc - 1][0];

    /* Alloco memoria per gli array di pipe */
    pipes_12 = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipes_21 = (pipe_t *)malloc(N * sizeof(pipe_t));

    /* Controllo che la malloc sia andata a buon fine per entrambe */
    if ((pipes_12 == NULL) || (pipes_21 == NULL))
    {
        printf("Errore nella malloc\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione della n-esima pipe tra primo e secondo figlio vada a buon fine */
        if (pipe(pipes_12[n]) < 0)
        {
            printf("Errore nella creazione della pipe tra primo e secondo figlio di indice n = %d\n", n);
            exit(4);
        }

        /* Controllo che la creazione della n-esima pipe tra secondo e primo figlio vada a buon fine */
        if (pipe(pipes_21[n]) < 0)
        {
            printf("Errore nella creazione della pipe tra secondo e primo figlio di indice n = %d\n", n);
            exit(5);
        }
    }

    /* Creo gli N*2 processi figli */
    for (n = 0; n < N * 2; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(6);
        }
        if (pid == 0)
        {
            if (n < N)
            {
                /* Primo processo figlio */
                /* Chiudo le pipe non necessarie */
                for (i = 0; i < N; i++)
                {
                    close(pipes_12[i][0]);
                    close(pipes_21[i][1]);
                    if (i != n)
                    {
                        close(pipes_21[i][0]);
                        close(pipes_12[i][1]);
                    }
                    
                }

                /* Apro in lettura il file */
                if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura di %s\n", argv[n + 1]);
                    exit(0);
                }
                
                /* Inizializzo occ a 0 */
                occ = 0;

                /* Itero un ciclo che legge un carattere alla volta il file */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto è quello cercato */
                    if (c == Cz)
                    {
                        /* Incremento occ di 1 */
                        occ++;

                        /* Torno indietro di una posizione e la comunico al secondo processo figlio */
                        pos = lseek(fd, 0L, SEEK_CUR) - 1L;
                        nw = write(pipes_12[n][1], &pos, sizeof(pos));
                        
                        /* Controllo che la write sia andata a buon fine */
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: Impossibile scrivere sulla pipe per il processo di indice %d\n", n);
                            exit(0);
                        }
                        
                        /* Aspetto dall'altro processo figlio la nuova posizione in caso contrario esco dal ciclo */
                        nr = read(pipes_21[n][0], &posLetta, sizeof(posLetta));
                        if (nr != sizeof(posLetta))
                        {
                            break;
                        }
                        
                        /* Imposto la posizione del file con la posizione letta da pipe */
                        lseek(fd, posLetta + 1L, SEEK_SET);
                    }
                }
                
            }
            else
            {
                /* Secondo processo figlio */
                /* Chiudo le pipe non necessarie */
                for (i = 0; i < N; i++)
                {
                    close(pipes_21[i][0]);
                    close(pipes_12[i][1]);
                    if (i != 2*N - n - 1)
                    {
                        close(pipes_12[i][0]);
                        close(pipes_21[i][1]);
                    }
                    
                }

                /* Apro il file associato al processo figlio */
                if ((fd = open(argv[2*N - n], O_RDONLY)) < 0)
                {
                    printf("Errore nella open del file %s\n", argv[2*N - n]);
                    exit(0);
                }
                
                /* Inizializzo occ a 0 */
                occ = 0;

                /* Leggo la prima posizione inviata dal primo processo figlio */
                nr = read(pipes_12[2*N - n - 1][0], &posLetta, sizeof(posLetta));
                if (nr != sizeof(posLetta))
                {
                    printf("Errore: Impossibile leggere dalla pipe per il processo di indice %d\n", n);
                    exit(0);
                }
                
                /* Sposto l'I/O pointer sulla posizione letta */
                lseek(fd, posLetta + 1L, SEEK_SET);

                /* Itero un ciclo che legge il file un carattere alla volta */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto è quello cercato */
                    if (c == Cz)
                    {
                        /* Incremento il numero di occorrenze */
                        occ++;

                        /* Assegno a pos la posizione del carattere e la invio al primo processo figlio */
                        pos = lseek(fd, 0L, SEEK_CUR) -1L;
                        nw = write(pipes_21[2*N - n - 1][1], &pos, sizeof(pos));

                        /* Controllo che la write sia andata a buon fine */
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: Impossibile scrivere sulla pipe per il processo di indice %d\n", n);
                            exit(0);
                        }
                        
                        /* Leggo la posizione inviata dal primo processo figlio in caso contrario esco dal ciclo */
                        nr = read(fd, &posLetta, sizeof(posLetta));
                        if (nr != sizeof(posLetta))
                        {
                            break;
                        }
                        
                        lseek(fd, posLetta + 1L, SEEK_SET);
                    }
                    
                }
                
            }

            exit(occ);
        }
    }

    /* Processo padre */
    /* Chiudo tutte le pipe */
    for (n = 0; n < N; n++)
    {
        close(pipes_12[n][0]);
        close(pipes_12[n][1]);
        close(pipes_21[n][0]);
        close(pipes_21[n][1]);
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N * 2; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            if (ritorno == 0)
            {
                printf("Il processo figlio con PID: %d ha ritornato %d quindi ha avuto problemi\n", pidFiglio, ritorno);
            }
            else
            {
                printf("Il processo figlio con PID: %d ha ritornato %d\n", pidFiglio, ritorno);
            }
        }
    }
    
    exit(0);
}