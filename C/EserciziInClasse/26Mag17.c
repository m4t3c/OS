#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco la struct strut */
typedef struct {
    long int max_occ;
    int index;
    long int sum;
} strut;

/* Definisci il tipo pipe_t come array di due interi */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file passati come parametro */
    char Cx;                        /* Caratteri passati come ultimo parametro */
    int fd;                         /* Per open */
    int *pid;                       /* Array di pid */
    pipe_t *pipes;                  /* Array di pipe di comunicazione */
    strut s;                        /* Struct che viene passata tra i figli e poi al padre */
    char c;                         /* Singolo carattere letto */
    long int curr_occ;              /* Numero di occorrenze di Cx nel file corrente */
    int i, j;                       /* Indici per i cicli */
    int nr, nw;                     /* Variabili di controllo per read e write */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel passaggio dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di file passati come parametro */
    N = argc - 2;

    /* Controllo che l'ultimo parametro passato sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore: %s non è un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Inizializzo Cx con il carattere passato come parametro */
    Cx = argv[argc - 1][0];

    /* Alloco memoria per l'array di pipe */
    if ((pipes = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(3);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazine della pipe di indice i = %d\n", i);
            exit(4);
        }
        
    }
    
    /* Alloco memoria per l'array di pid */
    if ((pid = (int *)malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(5);
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (j = 0; j < N; j++)
            {
                if (i == 0 || j != i - 1)
                {
                    close(pipes[j][0]);
                }
                if (i != j)
                {
                    close(pipes[j][1]);
                }
                
            }
            
            /* Apro il file in lettura */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[i + 1]);
                exit(N);
            }
            
            /* Inizializzo curr_occ a 0 */
            curr_occ = 0;

            /* Itero un ciclo che legge un carattere alla volta il file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == Cx)
                {
                    curr_occ++;
                }
                
            }

            /* Se sono al primo processo filgio inizializzo la struct in caso contrario recupero la struct da pipe */
            if (i == 0)
            {
                s.index = 0;
                s.max_occ = curr_occ;
                s.sum = curr_occ;
            }
            else
            {
                nr = read(pipes[i - 1][0], &s, sizeof(strut));
                if (nr != sizeof(strut))
                {
                    printf("Figlio %d ha letto un numero di byte sbagliati %d\n", i, nr);
        			exit(N+1);
                }
                
                /* Controllo se curr_occ è > di max_occ */
                if (curr_occ > s.max_occ)
                {
                    s.index = i;
                    s.max_occ = curr_occ;
                }
                
                s.sum += curr_occ;
            }
            
            /* Mando in avanti la struct al prossimo figlio/padre */
            nw = write(pipes[i][1], &s, sizeof(strut));
            if (nw != sizeof(strut))
            {
                printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
                exit(N+2);
            }
            
            exit(i);
        }

    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe eccetto l'ultima pipe in lettura */
    for (i = 0; i < N; i++)
    {
        if (i != N - 1)
        {
            close(pipes[i][0]);
        }
        close(pipes[i][1]);
    }
    
    /* Recupero le informazioni dall'ultima pipe */
    nr = read(pipes[N - 1][0], &s, sizeof(strut));
    if (nr != sizeof(strut))
    {
        printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
        exit(7);
    }
    
    /* Il padre stampa le informazioni della struct */
    printf("Sono stati letti in totale da tutti i processi figli %ld occorrenze del carattere %c\nIl processo figlio di indice %d e PID: %d ha letto dal file %s il numero massimo di occorrenze del carattere %c: %ld\n", s.sum, Cx, s.index, pid[s.index], argv[s.index + 1], Cx, s.max_occ);

    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
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