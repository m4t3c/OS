#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe come array di due interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file passati */
    char Cz;                        /* Variabile che salva il carattere passato come ultimo parametro */
    int pid;                        /* Per fork */
    pipe_t *pipes_12;               /* Array di pipe tra il primo figlio e il secondo associato allo stesso file */
    pipe_t *pipes_21;               /* Array di pipe tra il secondo figlio e il primo associato allo stesso file */
    int i, j;   	                /* Indici dei cicli */
    int fd;                         /* File descriptor per la open */
    char c;                         /* Buffer per la read */
    long int pos;                   /* Posizione del carattere cercato */
    long int pos_letta;             /* Posizione da cui partire con la lettura */
    int occ;                        /* Numero di caratteri trovati */
    int nr, nw;                     /* Per read e write */
    int ritorno, status;            /* Per wait */
    /* ------------------------------ */
    
    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, ..., Cz) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di file passati nella variabile N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }
    /* Salvo l'ultimo parametro nella variabile Cz */
    Cz = argv[argc - 1][0];

    /* Alloco memoria per gli array di pipe */
    pipes_12 = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipes_21 = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che le malloc siano andate a buon fine */
    if ((pipes_12 == NULL) || (pipes_21 == NULL))
    {
        printf("Errore nella malloc per gli array di pipe\n");
        exit(3);
    }
    
    /* Creo le N + N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipes_12[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra primo e secondo figlio di indice i = %d\n", i);
            exit(4);
        }
        
        if (pipe(pipes_21[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra secondo e primo filgio di indice i = %d\n", i);
            exit(5);
        }
    }
    
    /* Creo i 2*N processi figli */
    for (i = 0; i < 2*N; i++)
    {
        /* Controllo che la creazione dell'i-esimo figlio sia andata a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            /* Controllo se sono nel primo o nel secondo filgio associato all'i-esimo file */
            if (i < N)
            {
                /* Primo processo figlio */
                /* Si e' scelto di ritornare 0 in caso di errori nel processo filgio */
                /* Chiudo tutte le pipe che non mi servono */
                for (j = 0; j < N; j++)
                {
                    close(pipes_12[j][0]);
                    close(pipes_21[j][1]);
                    if (j != i)
                    {
                        close(pipes_12[j][1]);
                        close(pipes_21[j][0]);
                    }
                }
                
                /* Apro in lettura il file associato al processo figlio */
                if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                    exit(0);
                }
                
                /* Setto a 0 la variabile occ */
                occ = 0;

                /* Itero un ciclo che legge tutti i caratteri del file */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto e' uguale a quello cercato */
                    if (c == Cz)
                    {
                        /* Incremento il numero di occorrenze e comunico al secondo processo figlio la posizione da cui partire */\
                        occ++;
                        pos = lseek(fd, 0L, SEEK_CUR) - 1L;
                        nw = write(pipes_12[i][1], &pos, sizeof(pos));
                        /* Controllo che la write sia stata eseguita correttamente */
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: impossibile scrivere su pipe per il processo figlio di indice i = %d\n", i);
                            exit(0);
                        }
                        
                        /* Leggo dalla pipe la posizione inviata */
                        nr = read(pipes_21[i][0], &pos_letta, sizeof(pos_letta));
                        /* Controllo che la read sia stata eseguita correttamente in caso contrario interrompo il ciclo */
                        if (nr != sizeof(pos_letta))
                        {
                            break;
                        }
                        /* Setto l'I/O pointer alla posizione letta */
                        lseek(fd, pos_letta + 1L, SEEK_SET);
                    }
                }   
            }
            else
            {
                /* Secondo processo figlio */
                /* Si e' scelto di ritornare 0 in caso di errori nel processo filgio */
                /* Chiudo tutte le pipe che non mi servono */
                for (j = 0; j < N; j++)
                {
                    close(pipes_21[j][0]);
                    close(pipes_12[j][1]);
                    if (j != 2*N - i - 1)
                    {
                        close(pipes_21[j][1]);
                        close(pipes_21[j][0]);
                    }
                }
                
                /* Apro in lettura il file associato al processo figlio */
                if ((fd = open(argv[2*N - i], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura in lettura del file %s\n", argv[2*N - i]);
                    exit(0);
                }
                
                /* Inizializzo occ a 0 */
                occ = 0;
                
                /* Recupero la posizione dalla pipe del primo figlio e lo setto come posizione di partenza */
                nr = read(pipes_12[2*N - i - 1][0], &pos_letta, sizeof(pos_letta));
                if (nr != sizeof(pos_letta))
                {
                    printf("Errore: Impossibile leggere dalla pipe per il processo di indice i = %d (PRIMA LETTURA)\n", i);
                    exit(0);
                }
                lseek(fd, pos_letta + 1L, SEEK_SET);

                /* Itero un ciclo che legge tutti i caratteri del file associato al processo figlio */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto e' uguale a quello cercato */
                    if (c == Cz)
                    {
                        /* Aumento il numero di occorrenze e comunico al primo processo figlio la posizione */
                        occ++;
                        pos = lseek(fd, 0L, SEEK_CUR) - 1L;
                        nw = write(pipes_21[2*N - i - 1][1], &pos, sizeof(pos));
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: Impossibile scrivere nella pipe per il processo figlio di indice i = %d\n", i);
                            exit(0);
                        }
                        
                        /* Recupero dal primo processo figlio la posizione da cui ripartire */
                        nr = read(pipes_12[2*N - i - 1][0], &pos_letta, sizeof(pos_letta));
                        if (nr != sizeof(pos_letta))
                        {
                            break;
                        }
                        lseek(fd, pos_letta + 1L, SEEK_SET);
                    }
                }
            }
            
            exit(occ);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipes_12[i][0]);
        close(pipes_21[i][0]);
        close(pipes_12[i][1]);
        close(pipes_21[i][1]);
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < 2*N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d ha ritornato %d (se 0 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}