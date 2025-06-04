#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Definisco la struct Strut secondo le specifiche del testo */
typedef struct
{
    long int occMax;        /* Campo c1 del testo */
    int indMax;             /* Campo c2 del testo */
    long int sum;           /* Campo c3 del testo */
} Strut;

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file/processi figli */
    char Cx;                        /* Carattere passato come ultimo parametro */
    int fd;                         /* File descriptor per il file associato al processo figlio */
    int *pid;                       /* Array di pid per fork */
    pipe_t *pipes;                  /* Array di pipe */
    int i, j;                       /* Indici dei cicli */
    char c;                         /* Buffer del carattere letto */
    Strut S;                        /* Struct che contiene le informazioni richieste */
    long int occ = 0L;              /* Occorrenze del processo figlio */
    int nr, nw;                     /* Per read e write */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, ..., Cx) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di file nella variabile N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Salvo l'ultimo parametro nella variabile Cx */
    Cx = argv[argc - 1][0];

    /* Alloco memoria per l'array di pipe */
    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(3);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esima pipe sia andata a buon fine */
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(4);
        }
    }
    
    /* Alloco memoria per l'array di pid */
    pid = (int *)malloc(N * sizeof(int));
    /* Controllo che la malloc sia andata a buon fine */
    if (pid == NULL)
    {
        printf("Errore nella malloc dell'array di pid\n");
        exit(5);
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork sia andata a buon fine */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo tutte le pipe che non mi servono */
            for (j = 0; j < N; j++)
            {
                if (j != i)
                {
                    close(pipes[j][1]);
                }
                if ((i == 0) || (j != i - 1))
                {
                    close(pipes[j][0]);
                }
                
            }
            
            /* Se non sono nel primo processo figlio recupero le informazioni dalla pipe in lettura */

            /* Apro in lettura il file associato al processo figlio */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Setto a 0 j che fungera' da indice per la read */
            j = 0;

            /* Itero un ciclo che legge tutti i caratteri del file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto e' quello cercato */
                if (c == Cx)
                {
                    /* Incremento il numero di occorrenze trovate in questo processo figlio */
                    occ++;
                }
            }
            
            /* Distinguo in due casi: sono o non sono nel primo processo figlio */
            if (i == 0)
            {
                S.indMax = 0;
                S.occMax = occ;
                S.sum = occ;
            }
            else
            {
                nr = read(pipes[i - 1][0], &S, sizeof(Strut));
                if (nr != sizeof(S))
                {
                    printf("Figlio %d ha letto un numero errato di byte %d\n", i, nr);
                    exit(-1);
                }
                if (occ > S.occMax)
                {
                    S.occMax = occ;
                    S.indMax = i;
                }
                S.sum += occ;
            }

            /* Mando al prossimo processo figlio/processo padre le informazioni */
            nw = write(pipes[i][1], &S, sizeof(Strut));
            if (nw != sizeof(Strut))
            {
                printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
                exit(-1);
            }
            
            /* Ritorno l'indice d'ordine */
            exit(i);
        }
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        close(pipes[i][1]);
        if (i != N - 1)
        {
            close(pipes[i][0]);
        }
    }
    
    /* Il padre recupera le informazioni dall'ultima pipe */
    nr = read(pipes[N - 1][0], &S, sizeof(Strut));
    if (nr != sizeof(Strut))
    {
        printf("Processo padre ha letto numero di byte sbagliati %d\n", nr);
    }
    else
    {
        printf("Il figlio di indice %d e pid %d ha trovato il numero massimo di occorrenze %ld del carattere %c nel file %s\n", S.indMax, pid[S.indMax], S.occMax, Cx, argv[S.indMax + 1]);
        printf("I vari processi figli hanno trovato in totale %ld occorrenze del carattere %c nei file\n", S.sum, Cx);
    }

    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' ritornato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}