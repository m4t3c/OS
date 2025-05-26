#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];                  /* Array di due interi che ci servirà per le N pipe */

typedef struct
{
    int index;                          /* Indice di processo */
    long int occ;                       /* Numero di occorrenze */
} s_occ;


int main(int argc, char **argv)
{
    /*----- Variabili locali -----*/
    int N;                              /* Numero di file passati */
    char Cx;                            /* Carattere da cercare */
    int i, j;                           /* Indici per i cicli */
    int *pid;                           /* Array dei PID */
    int fd;                             /* Per la open */
    int nr;                             /* Variabile che salva il numero di dati letti dalla pipe del figlio precedente */
    int nw;                             /* Variabile che salva il numero di dati scritti sulla pipe per il filgio successivo */
    pipe_t *piped;                      /* Array di pipe */
    s_occ *cur;                         /* Array di struct */
    char c;                             /* Buffer della read */
    int pidFiglio, ritorno, status; 	/* Per wait */
    /*----------------------------*/

    if(argc < 4)                        /* Controllo che siano passati almeno 3 parametri */
    {
        printf("Errore nel numero di parametri: ho bisogno di almeno 3 parametri (N file e 1 carattere Cx) ma argc = %d\n", argc);
        exit(1);
    }

    if(strlen(argv[argc - 1]) != 1)     /* Controllo che l'ultimo carattere sia un singolo carattere */
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }

    Cx = argv[argc - 1][0];

    if(!islower(Cx))                     /* Controllo che il carattere sia minuscolo */
    {
        printf("Errore nel passaggio dei parametri: %c non e' un carattere alfabetico minuscolo\n", Cx);
        exit(3);
    }

    N = argc - 2;                       /* Assegno ad N il numero di file passati come parametri */

    /* Alloco spazio per l'array dei PID */
    pid = (int *)malloc(N * sizeof(int));
    if(pid == NULL)                     /* Controllo che l'array dei pid sia stato allocato con successo */
    {
        printf("Errore nella malloc per l'array di pid\n");
        exit(4);
    }
    
    /* Alloco spazio per N pipe */
    piped = (pipe_t *)malloc(N * sizeof(pipe_t));
    if(piped == NULL)                   /* Controllo che l'array di pipe sia stato allocato con successo */
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(5);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if(pipe(piped[i]) < 0)          /* Controllo che la creazione dell'i-esima pipe vada a buon fine */
        {
            printf("Errore nella creazione di pipe di indice i = %d\n", i);
            exit(6);
        }
    }
    
    /* Creo N processi figli */
    for (i = 0; i < N; i++)
    {
        if((pid[i] = fork()) < 0)       /* Controllo che la creazione dell'i-esimo processo figlio vada a buon fine */
        {
            printf("Errore nella fork del processo figlio numero %d\n", i);
            exit(7);
        }
        if(pid[i] == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe che non mi servono */
            for (j = 0; j < N; j++)
            {
                if((i == 0) || (j != i - 1))
                {
                    close(piped[j][0]);
                }
                if(i != j)
                {
                    close(piped[j][1]);
                }
            }
            
            /* Alloco spazio per i + 1 struct */
            cur = (s_occ *) malloc((i + 1) * sizeof(s_occ));
            if(cur == NULL)         /* Controllo che l'allocazione abbia avuto successo */
            {
                printf("Errore nell'allocazione cur per il figlio di indice i = %d\n", i);
                exit(-1);
            }

            /* Apro in lettura il file passato come i-esimo parametro */
            if((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'appertura del file %s\n", argv[i + 1]);
                exit(-2);
            }
            /* Inizializzo la struct */
            cur[i].index = i;
            cur[i].occ = 0L;

            while (read(fd, &c, 1) > 0)
            {
                if(c == Cx)         /* Controllo che il carattere letto sia uguale a quello cercato */
                {
                    cur[i].occ++;
                }
            }

            /* Nel caso non sia al primo processo figlio leggo dalla pipe precedente */
            if(i != 0)
            {
                nr = read(piped[i - 1][0], cur, i * sizeof(s_occ));
                if(nr != i * sizeof(s_occ))     /* Controllo che il numero di dati letti sia uguale alla grandezza delle struct */
                {
                    printf("Il figlio di indice i = %d ha letto un numero di strutture sbagliate %d\n", i, nr);
                    exit(-3);
                }               
            }

            /* Scrivo sulla pipe per il processo figlio successivo */
            nw = write(piped[i][1], cur, (i + 1)*sizeof(s_occ));
            if(nw != ((i + 1)*sizeof(s_occ)))
            {
                printf("Il figlio di indice i = %d ha scritto un numero di strutture sbagliate%d\n", i, nw);
                exit(-4);
            }
            
            exit(i);
        }
    }

    /* Processo padre */
    /* Chiudo tutte le pipe che non mi servono */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
        if(i != N - 1)
        {
            close(piped[i][0]);
        }
        
    }

    /* Alloco una struct di dimensione N (quanto i figli) per il processo padre */
    cur = (s_occ *) malloc(N * sizeof(s_occ));
    if(cur == NULL)         /* Controllo che l'allocazione della struct abbia avuto successo */
    {
        printf("Errore nella malloc di cur nel padre\n");
        exit(8);   
    }

    nr = read(piped[N - 1][0], cur, N*sizeof(s_occ));
    if(nr != N * sizeof(s_occ))     /* Controllo che siano stati letti N dati dalla pipe */
    {
        printf("Il processo padre ha letto un numero di strutture sbagliate%d\n", nr);
        exit(9);
    }
    else
    {
        nr /= sizeof(s_occ);
        for (i = 0; i < N; i++)
        {
            printf("Il figlio di indice i = %d con PID %d ha trovato %ld occorrenze del carattere '%c' nel file %s\n", cur[i].index, pid[cur[i].index], cur[i].occ, Cx, argv[cur[i].index + 1]);
        }
    }

    /* Ciclo per aspettare i processi figli */
    for (i = 0; i < N; i++)
    {
        if((pidFiglio = wait(&status)) < 0)         /* Controllo che la wait vada a buon fine */
        {
            printf("Errore nella wait\n");
            exit(10);
        }
        else if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID = %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID = %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}