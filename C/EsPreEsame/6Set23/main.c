#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi*/
typedef int pipe_t[2];

/* Definisco il tipo Strut come struttura che contiene un campo long int e un campo int */
typedef struct
{
    long int minLinee;      /* Campo c1 del testo */
    int indOrdine;          /* Campo c2 del testo */
} Strut;


int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di parametri/processi figli */
    int *pid;                       /* Array di pid */
    int fd;                         /* File descriptor per la open */
    pipe_t *pipes;                  /* Array di pipe di comunicazione tra i figli */
    pipe_t *pipe_pf;                /* Array di pipe di comunicazione tra padre e figlio */
    Strut S;                        /* Struttura che contiene i dati richiesti dal testo */
    char linea[250];                /* Buffer della linea letta */
    int n, i;                       /* Indici per i cicli */
    int cur_tot = 0;                /* Counter del numero di linee */
    int nr, nw;                     /* Per read e write */
    long int min;                   /* Numero minimo di linee trovate */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, nomefile2, ...) ma argc = %d", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri in N */
    N = argc - 1;

    /* Alloco memoria per gli array di pipe */
    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che l'allocazione sia andata a buon fine */
    if ((pipes == NULL) || (pipe_pf == NULL))
    {
        printf("Errore nella malloc degli array di pipe\n");
        exit(2);
    }
    
    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        if (pipe(pipes[n]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra i figli di indice n = %d\n", n);
            exit(3);
        }
        
        if (pipe(pipe_pf[n]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra padre e figlio di indice n = %d\n", n);
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
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[n] = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(6);
        }
        if (pid[n] == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo tutte le pipe non necessarie */
            for (i = 0; i < N; i++)
            {
                close(pipe_pf[i][1]);
                if (i != n)
                {
                    close(pipe_pf[i][0]);
                    close(pipes[i][1]);
                }
                if ((n == 0) || (i != n - 1))
                {
                    close(pipes[i][0]);
                }
            }
            
            /* Apro il file associato in lettura */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettrura del file %s\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Inizializzo a zero la variabile i che fungera' da indice nella read */
            i = 0;
            /* Itero un ciclo che legge tutti i caratteri del file */
            while (read(fd, &(linea[i]), 1))
            {
                /* Controllo se sono arrivato alla fine della linea */
                if (linea[i] == '\n')
                {
                    /* Incremento il numero di linee */
                    cur_tot++;
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
            
            /* Controllo se sono o meno nel primo processo figlio */
            if (n == 0)
            {
                /* Inizializzo la struct con i dati trovati */
                S.minLinee = cur_tot;
                S.indOrdine = n;
            }
            else
            {
                /* Recupero i dati passati dal precedente processo figlio */
                nr = read(pipes[n - 1][0], &S, sizeof(S));
                if (nr != sizeof(S))
                {
                    printf("Figlio %d ha letto un numero di byte errato %d\n", n, nr);
                    exit(-1);
                }
                
                /* Controllo se il file corrente ha meno righe del minimo precedentemente calcolato */
                if (cur_tot < S.minLinee)
                {
                    S.minLinee = cur_tot;
                    S.indOrdine = n;
                }
            }

            /* Scrivo sulla pipe successiva la struct aggiornata */
            nw = write(pipes[n][1], &S, sizeof(S));
            if (nw != sizeof(S))
            {
                printf("Figlio %d ha letto un numero di byte errato %d\n", n, nw);
                exit(-1);
            }
            
            /* A questo punto si attende il calcolo del minimo da parte del padre */
            /* Faccio ripartire da 0 il file */
            lseek(fd, 0L, 0);
            /* Leggo dal padre il valore minimo di linee */
            read(pipe_pf[n][0], &min, sizeof(min));
            
            /* Rinizializzo a 0 il numero di linee e l'indice i e ricomincio la lettura del file */
            cur_tot = 0;
            i = 0;

            while (read(fd, &(linea[i]), 1))
            {
                if (linea[i] == '\n')
                {
                    cur_tot++;

                    /* Controllo se sono arrivato alla linea minima */
                    if (cur_tot == min)
                    {
                        linea[i] = '\0';
                        printf("Processo figlio di indice n = %d e PID: %d ha letto la seguente linea di indice %ld nel file %s: %s\n", n, getpid(), min, argv[n + 1], linea);
                        break;
                    }
                    else
                    {
                        i = 0;
                    }
                }
                else
                {
                    i++;
                }
            }
            
            exit(n);
        }
    }
    
    /* Processo padre */
    /* Chiudo le pipe che non mi servono */
    for (n = 0; n < N; n++)
    {
        close(pipe_pf[n][0]);
        close(pipes[n][1]);
        if (n != N - 1)
        {
            close(pipes[n][0]);
        }
    }
    
    /* Recupero la struct dall'ultimo processo figlio */
    nr = read(pipes[N - 1][0], &S, sizeof(S));
    /* Controllo che sia stato letto un numero corretto di parametri */
    if (nr != sizeof(S))
    {
        printf("Il processo padre ha letto un numero errato di bytes %d\n", nr);
        exit(7);
    }
    
    /* Salvo il numero minimo di linee in min */
    min = S.minLinee;

    /* Stampo su standard output i dati della struct */
    printf("Il processo figlio di indice n = %d e PID: %d ha letto il file %s che ha il minor numero di linee: %ld\n", S.indOrdine, pid[S.indOrdine], argv[S.indOrdine + 1], min);

    /* Invio a tutti i figli il numero minimo di linee */
    for (n = 0; n < N; n++)
    {
        write(pipe_pf[n][1], &min, sizeof(min));
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF))
        {
            printf("Processo figlio con PID: %d e' terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}