#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo strut come struttura dati */
typedef struct {
    int pidn;      /* Pid del nipote */
    int nlinea;    /* Numero della linea */
    char line[250];/* Linea ricevuta dal nipote */
} strut;

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Definisco la funzione mia_random */
int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    casuale++;
    return casuale;
}

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di file passati come parametro */
    int pid;                        /* Per fork */
    pipe_t *pipes;                  /* Array di pipe tra padre e figlio */
    pipe_t p;                       /* Pipe tra figlio e nipote */
    int i, j;                       /* Indici per i cicli */
    int X, r;                       /* Numero di linee del file e numero random generato */
    strut S;                        /* Struct in cui */
    int nr;                         /* Variabile di controllo sulla read */
    char opzione[5];                /* Opzione del comando head */
    int finito = 0;                 /* Variabile di controllo che afferma se ci sono altre pipe da leggere */
    int nrlinee;                    /* Variabile usata da ogni figlio per calcolare le linee inviate dal nipote */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri e che siano pari */
    if ((argc < 3) || ((argc - 1) % 2))
    {
        printf("Errore nel passaggio dei parametri: ho bisogno di almeno 2 parametri e che siano pari ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di file passati come parametro */
    N = (argc - 1) / 2;

    /* Controllo che i parametri in posizioni pari siano numeri strettamente positivi */
    for (i = 0; i < N; i++)
    {
        if (argv[(i * 2) + 2][0] == '-' || argv[(i * 2) + 2][0] == '0')
        {
            printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[(i + 2) + 2]);
            exit(2);
        }
        
    }
    
    /* Alloco memoria per l'array di pipe */
    if ((pipes = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(3);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(4);
        }
        
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(5);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (j = 0; j < N; j++)
            {
                close(pipes[j][0]);
                if (i != j)
                {
                    close(pipes[j][1]);
                }
                
            }

            /* Creo la pipe di comunicazione tra figlio e nipote */
            if (pipe(p) < 0)
            {
                printf("Errore nella creazione della pipe di comunicazione tra figlio e nipote\n");
                exit(-1);
            }
            
            /* Creo il processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote\n");
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Chiudo le pipe non necessarie */
                close(pipes[i][1]);

                /* Simulo il piping dei comandi reindirizzando standard output su p[1] */
                close(1);
                dup(p[1]);

                /* Chiudo le pipe di comunicazione tra processo figlio e nipote */
                close(p[0]);
                close(p[1]);

                /* Inizializzo il seme della funzione mia_random */
                srand(time(NULL));

                /* Inizializzo X con il numero di linee del file */
                X = atoi(argv[(i * 2) + 2]);

                /* Genero il numero casuale r */
                r = mia_random(X);

                /* Costruisco la stringa per il comando head */
                sprintf(opzione, "-%d", r);

                /* Eseguo il comando head */
                execlp("head", "head", opzione, argv[(i * 2) + 1], (char *)0);

                /* Non si dovrebbe arrivare quì */
                perror("Errore nell'esecuzione del comando head");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Chiudo la pipe in scrittura tra figlio e nipote */
            close(p[1]);

            /* Inizializzo a 0 j, il valore del pid del nipote a S.pidn e nrlinee a 0 */
            j = 0;
            S.pidn = pid;
            nrlinee = 0;

            /* Itero un ciclo che legge un carattere alla volta dalla pipe */
            while (read(p[0], &(S.line[j]), 1))
            {
                /* Controllo se sono arrivato alla fine della linea */
                if (S.line[j] == '\n')
                {
                    nrlinee++;
                    j++;
                    S.line[j] = '\0';
                    S.nlinea = nrlinee;

                    /* Comunico al padre la seguente linea */
                    write(pipes[i][1], &S, sizeof(S));
                    j = 0;
                }
                else
                {
                    j++;
                }
            }
            
            /* Imposto di default ritorno a -1 */
            ritorno = -1;

            /* Il figlio aspetta i nipoti */
            if ((pidFiglio = wait(&status)) < 0)
            {
                printf("Errore nella wait del processo nipote\n");
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d terminato in modo anomalo\n", pidFiglio);
            }
            else
            {
                ritorno = nrlinee;
            }

            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        close(pipes[i][1]);
    }
    
    /* Itero un ciclo che termina quando finito = 1 */
    while (!finito)
    {
        /* Inizializzo finito a 1 */
        finito = 1;

        /* Itero un ciclo che recupera da ogni figlio una struct */
        for (i = 0; i < N; i++)
        {
            nr = read(pipes[i][0], &S, sizeof(S));
            if (nr != 0)
            {
                finito = 0;
                printf("Il nipote con pid %d ha letto dal file %s nella riga %d questa linea:\n%s", S.pidn, argv[(i*2) + 1], S.nlinea, S.line);
            }
            
        }
        
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(6);
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