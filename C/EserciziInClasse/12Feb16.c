#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco la struttura strut */
typedef struct {
    int index;
    long int occ;
} strut;

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di file passati come parametro */
    int fd;                         /* Per open */
    char Cx;                        /* Carattere passto come ultimo parametro */
    int *pid;                       /* Array di PID per fork */
    pipe_t *pipes;                  /* Array di pipe */
    strut *s;                       /* Array di strutture */
    int i, j;                       /* Indici per i cicli */
    char c;                         /* Singolo carattere letto */
    int nr, nw;                     /* Controlli per read e write */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 caratteri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di file passati come parametro */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if ((strlen(argv[argc - 1])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo parametro\n", argv[argc - 1]);
        exit(2);
    }

    /* Inizializzo Cx con l'ultimo parametro */
    Cx = argv[argc - 1][0];

    /* Controllo che l'ultimo parametro sia minuscolo */
    if (!islower(Cx))
    {
        printf("Errore nel passaggio dei parametri: %c non è un carattere minuscolo\n", Cx);
        exit(3);
    }
    
    
    /* Alloco memoria per l'array di pipe */
    if ((pipes = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(4);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che l'n-esima pipe sia creata correttamente */
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(5);
        }
        
    }

    /* Alloco memoria per l'array di PID */
    if ((pid = (int *)malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella malloc per l'array di PID\n");
        exit(6);
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(7);
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
            
            /* Alloco memoria per l'array di strutture */
            if ((s = (strut *)malloc((i + 1) * sizeof(strut))) == NULL)
            {
                printf("Errore nella malloc per l'array di strutture\n");
                exit(N);
            }

            /* Apro il file in lettura */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[i + 1]);
                exit(N + 1);
            }
            
            /* Inizializzo i valori della struct corrente */
            s[i].index = i;
            s[i].occ = 0L;

            /* Itero un ciclo che legge il file un carattere alla volta */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è uguale a quello cercato */
                if (c == Cx)
                {
                    (s[i].occ)++;
                }
                
            }
            
            /* Nel caso non fossimo nel primo processo figlio leggo da pipe */
            if (i != 0)
            {
                nr = read(pipes[i - 1][0], s, i * sizeof(strut));
                if (nr != (i * sizeof(strut)))
                {
                    printf("Figlio %d ha letto un numero sbagliato di strutture %d\n", i, nr);
                    exit(N + 2);
                }
                
            }
            
            /* Scrivo su pipe le (i + 1) strutture */
            nw = write(pipes[i][1], s, (i + 1) * sizeof(strut));
            if (nw != (i + 1) * sizeof(strut))
            {
                printf("Figlio %d ha scritto un numero sbagliato di strutture %d\n", i, nw);
                exit(N + 3);
            }
            
            exit(i);
        }
                
    }

    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        if (i != N - 1)
        {
            close(pipes[i][0]);
        }
        close(pipes[i][1]);
    }
    
    /* Alloco memoria per l'ultima struct */
    if ((s = (strut *)malloc(N * sizeof(strut))) == NULL)
    {
        printf("Errore nella malloc dell'array di struct\n");
        exit(8);
    }
    
    /* Leggo da pipe l'ultima struct */
    nr = read(pipes[N - 1][0], s, N*sizeof(strut));
    if (nr != N*sizeof(strut))
    {
        printf("Padre ha letto un numero di strutture sbagliate %d\n", nr);
        exit(9);
    }
    
    /* Stampo le informazioni scritte nella struttura */
    for (i = 0; i < N; i++)
    {
        printf("Il processo figlio di indice %d e PID: %d ha trovato %ld occorrenze del carattere %c nel file %s\n", s[i].index, pid[s[i].index], s[i].occ, Cx, argv[s[i].index + 1]);
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(10);
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