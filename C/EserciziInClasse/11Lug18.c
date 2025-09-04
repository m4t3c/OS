#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* VARIABILI GLOBALI */

int *finito; /* array dinamico per indicare i figli che sono terminati */

int N; /* Numero di file passati come primo parametro */

int finitof()
{
    /* questa funzione verifica i valori memorizzati nell'array finito: appena trova un elemento uguale a 0 vuole dire che non tutti i processi figli sono finiti e quindi torna 0; tornera' 1 se e solo se tutti gli elementi dell'array sono a 1 e quindi tutti i processi figli sono finiti */
    int i;
    for (i = 0; i < N; i++)
        if (!finito[i])
            /* appena ne trova uno che non ha finito */
            return 0; /* ritorna falso */
    return 1;
}

int main(int argc, char **argv)
{

    /* ------ Variabili Locali ------ */
    char CZ;                        /* Carattere passato come primo parametro */
    int pid;                        /* Per fork */
    int fd;                         /* Per open */
    pipe_t *pipes_pf;               /* Array di pipe tra padre e figlio */
    pipe_t *pipes_fp;               /* Array di pipe tra figlio e padre */
    int i, j;                       /* Indici per i cicli */
    int nr, nw;                     /* Variabili di controllo per read e write */
    int occ;                        /* Numero di occorrenze del carattere Cz */
    int index;                      /* Indice del processo filgio che ha inviato la posizione massima */
    char c, chControllo;            /* Carattere letto e carattere inviato dal padre per indicare al figlio di stampare o meno */
    long int pos;                   /* Posizione del carattere Cz */
    long int pos_max;               /* Posizione massima trovata */
    int ritorno, pidFiglio, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che il primo parametro sia un singolo carattere */
    if ((strlen(argv[1])) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[1]);
        exit(2);
    }
    
    /* Inizializzo CZ con il primo parametro */
    CZ = argv[1][0];

    /* Inizializzo N con il restante numero di parametri passati */
    N = argc - 2;

    /* Alloco memoria per gli array di pipe */
    pipes_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipes_fp = (pipe_t *)malloc(N * sizeof(pipe_t));

    /* Controllo che la malloc sia andata a buon fine */
    if ((pipes_pf == NULL) || (pipes_fp == NULL))
    {
        printf("Errore nella malloc per gli array di pipe\n");
        exit(3);
    }


    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipes_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra padre e figlio di indice i = %d\n", i);
            exit(4);
        }
        
        if (pipe(pipes_fp[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra figlio e padre di indice i = %d\n", i);
            exit(5);
        }
        
    }

    /* Alloco memoria per l'array finito */
    finito = (int *)malloc(N * sizeof(int));
    if (finito == NULL)
    {
        printf("Errore nella malloc dell'array finito\n");
        exit(6);
    }

    /* Inizializzo l'array a 0 */
    memset(finito, 0, N * sizeof(int));
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(7);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (j = 0; j < N; j++)
            {
                close(pipes_fp[j][0]);
                close(pipes_pf[j][1]);
                if (j != i)
                {
                    close(pipes_pf[j][0]);
                    close(pipes_fp[j][1]);
                }
                
            }
            
            /* Apro il file associato in lettura */
            if ((fd = open(argv[i + 2], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[i + 2]);
                exit(-1);
            }

            /* Inizializzo j e occ a 0 */
            j = 0;
            occ = 0L;

            /* Itero un ciclo che legge un carattere alla volta dal file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == CZ)
                {
                    /* Incremento il numero di occorrenze trovate */
                    occ++;

                    /* Invio la posizione al padre */
                    nw = write(pipes_fp[i][1], &pos, sizeof(pos));
                    if (nw != sizeof(pos))
                    {
                        printf("Errore: processo figlio di indice i = %d ha scritto un numero errato di byte %d\n", i, nw);
                        exit(-1);
                    }
                    /* Aspetto il segnale dal padre */
                    nr = read(pipes_pf[i][0], &chControllo, 1);
                    if (nr != 1)
                    {
                        printf("Errore: processo figlio di indice i = %d ha letto un numero errato di byte %d\n", i, nr);
                        exit(-1);
                    }
                    
                    /* Controllo se il carattee inviato dal padre è S, nel caso stampo, in caso sia N non stampo */
                    if (chControllo == 'S')
                    {
                        printf("Il carattere %c è stato trovato dal processo figlio di indice i = %d e PID: %d in posizione %ld nel file %s\n", CZ, i, getpid(), pos, argv[i + 2]);
                    }
                    
                }
                else
                {
                    pos++;
                }
            }
            
            exit(occ);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        close(pipes_pf[i][0]);
        close(pipes_fp[i][1]);
    }
    
    /* Il padre itera un ciclo che termina quando tutti i processi figli sono terminati */
    while (!finitof())
    {
        /* Inizializzo pos_max a -1 */
        pos_max = -1;

        /* Itero un ciclo che recupera le informazioni da tutti i processi figli */
        for (i = 0; i < N; i++)
        {
            /* Finito prenderà il valore 1 se non si è letto il numero corretto di bytes */
            finito[i] = (read(pipes_fp[i][0], &pos, sizeof(pos)) != sizeof(pos));

            /* Controllo se la poszione massima è minore di quella letta */
            if (pos > pos_max)
            {
                /* Aggiorno le variabili index e pos_max */
                pos_max = pos;
                index = i;
            }
            
        }
        
        /* Invio ai processi figli l'indicazione di stampare o meno le informazioni su standard output */
        for (i = 0; i < N; i++)
        {
            if (i == index)
            {
                chControllo = 'S';
            }
            else
            {
                chControllo = 'N';
            }
            if (!finito[i])
            {
                nw = write(pipes_pf[i][1], &chControllo, 1);
                if (nw != 1)
                {
                    printf("Processo padre ha scritto un numero errato di byte al figlio di indice i = %d\n", i);
                }

            }
            
        }
        
    }

    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
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