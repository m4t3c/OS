#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2]; /* Struct di due interi per le pipe */

/* Variabili globali */

int *finito; /* array dinamico che sta ad indicare se i figli sono terminati */
int N;       /* Numero di processi figli */

int finitof()
{
    /* questa funzione verifica i valori memorizzati nell'array finito: appena trova un elemento uguale a 0 vuole dire che non tutti i processi figli sono finiti e quindi torna 0; tornera' 1 se e solo se tutti gli elementi dell'array sono a 1 e quindi tutti i processi sono finiti */
    int i;
    for (i = 0; i < N; i++)
        if (!finito[i])
            /* appena ne trova uno che non ha finito */
            return 0; /* ritorna falso */
    return 1;
}

int main(int argc, char **argv)
{
    /*------ Variabili locali ------*/
    int pid;                        /* Per fork */
    char Cx;                        /* Carattere da cercare nei file */
    pipe_t *pipe_pf;                /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t *pipe_fp;                /* Array di pipe di comunicazione tra figlio e padre */
    int i, j;                       /* Indici per i cicli */
    int fd;                         /* Per la open del i-esimo file */
    long int posizione;             /* Variabile che comunica al padre la posizione del carattere */
    long int posizioneMax;          /* usata dal padre per calcolare il minimo */
    int occorrenze;                 /* Variabile che conta il numero di occorrenze del carattere nel file */
    char c;                         /* Buffer della read */
    int nw, nr;                     /* Per write e read */
    int indice;                     /* Variabile per tenere traccia del figlio di cui e' stato calcolato il massimo nel padre */
    char chControllo;               /* Variabile che contiene il segnale da parte del padre se bisogna scrivere o meno*/
    int pidFiglio, ritorno, status; /* Per wait */
    /*-------------------------------*/

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero di parametri: ho bisogno di almeno 3 parametri (carattere, N nomi file) ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che il primo parametro sia un singolo carattere */
    if ((strlen(argv[1])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[1]);
        exit(2);
    }

    Cx = argv[1][0];
    N = argc - 2;

    /* Alloco memoria per l'array finito */
    finito = (int *)malloc(N * sizeof(int));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (finito == NULL)
    {
        printf("Errore nella malloc\n");
        exit(3);
    }

    /* Setto l'array a 0 */
    memset(finito, 0, N * sizeof(int));

    /* Alloco memoria per N pipe */
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipe_fp = (pipe_t *)malloc(N * sizeof(pipe_t));

    /* Controllo che la malloc abbia avuto successo per le pipe */
    if ((pipe_pf == NULL) || (pipe_fp == NULL))
    {
        printf("Errore nella malloc\n");
        exit(4);
    }

    /* Inizializzo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che le creazioni delle pipe vadano a buon fine */
        if ((pipe(pipe_fp[i])) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra figlio e padre di indice i = %d\n", i);
            exit(5);
        }

        if ((pipe_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra padre e figlio di indice i = %d\n", i);
            exit(6);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esimo processo figlio abbia successo */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(7);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo tutti i lati delle pipe che non mi servono */
            for (j = 0; i < N; i++)
            {
                close(pipe_fp[j][0]);
                close(pipe_pf[j][1]);
                if (i != j)
                {
                    close(pipe_fp[j][1]);
                    close(pipe_pf[j][0]);
                }
            }
            /* Apriamo il file associato al processo figlio */
            if ((fd = open(argv[i + 2], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file %s\n", argv[i + 2]);
                exit(-1);
            }

            /* Inizializzo le variabili posizione e occorrenze */
            posizione = 0L;
            occorrenze = 0;
            /* Itero un ciclo finchè leggo caratteri dal file */
            while (read(fd, &c, 1))
            {
                if (c == Cx)
                {
                    occorrenze++;
                    nw = write(pipe_fp[i][1], &posizione, sizeof(posizione));
                    if (nw != sizeof(posizione))
                    {
                        printf("Errore nella write sulla pipe figlio padre del processo di indice %d\n", i);
                        exit(-1);
                    }

                    /* Leggo dal padre se bisogna scrivere o meno */
                    nr = read(pipe_pf[i][0], &chControllo, 1);
                    if (nr != 1)
                    {
                        printf("Errore nella read sulla pipe padre figlio del processo di indice i = %d\n", i);
                        exit(-1);
                    }

                    if (chControllo == 'S')
                    {
                        printf("Il figlio di indice %d e pid %d ha trovato un'occorrenza del carattere '%c' nel file %s in poszione %ld\n", i, pid, Cx, argv[i + 2], posizione);
                    }
                }
            }

            posizione++;

            exit(occorrenze);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe di cui non ho bisogno */
    for (i = 0; i < N; i++)
    {
        close(pipe_fp[i][1]);
        close(pipe_pf[i][0]);
    }

    while (!finitof())
    {
        posizioneMax = -1L;
        for (i = 0; i < N; i++)
        {
            /* Inizializzo finito[i] a 0 se abbiamo letto dei caratteri a 1 se non sono stati letti dei caratteri */
            finito[i] = (read(pipe_fp[i][0], &posizione, sizeof(posizione)) != sizeof(posizione));

            if (!finito[i])
            {
                if (posizione > posizioneMax)
                {
                    posizioneMax = posizione;
                    indice = i;
                }
            }
        }

        /* Al figlio in cui abbiamo trovato il massimo diciamo di stampare sennò diciamo di non stampare */
        for (i = 0; i < N; i++)
        {
            if (indice == i)
            {
                chControllo = 'S';
            }
            else
            {
                chControllo = 'N';
            }

            /* Scrivo solo ai figli non finiti */
            if (!finito[i])
            {
                nw = write(pipe_pf[i][1], &chControllo, 1);
                if (nw != 1)
                {
                    printf("Errore nella write nel processo padre per il processo figlio i = %d\n", i);
                }
            }
        }
    }

    /* Attesa della terminazione dei figli */
    for (i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        else if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d e' ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }

    exit(0);
}