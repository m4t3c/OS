#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

typedef int pipe_t[2];          /* Array di 2 interi per le pipe */      

typedef struct                  /* Struttura che contiene: */
{
    int pid_nipote;             /* PID del processo nipote */
    int n_linea;                /* Numero di linea letta */
    char linea[250];            /* Linea letta */
} strut;

int mia_random(int n)           /* funzione che calcola un numero random compreso fra 1 e n (fornita nel testo) */
{ 	
    int casuale;
	casuale = rand() % n;
	casuale++;
	return casuale;
}

int main(int argc, char **argv)
{
    /* -------- Variabili locali ---------- */
    int N;                      /* Numero di file passati */
    int X;                      /* Numero di righe del file */
    int i, j;                   /* Indici dei cicli */
    int pid;                    /* Variabile per la fork */
    int r;                      /* Numero casuale generato dalla funzione */
    pipe_t *piped;              /* Array di N pipe tra padre e figlio */
    pipe_t p;                   /* Pipe del processo nipote */
    char opzione[5];            /* Stringa che fungera' da opzione nel comando head */
    int nrLinee;                /* Numero di linea letta */
    strut S;                    /* Struttura usata dai filgi e dal padre */
    int status, ritorno;        /* Per wait */
    int nr;                     /* Per read */
    int finito;                 /* Variabile di controllo */

    /* ------------------------------------ */
    
    /* Controllo che i parametri siano almeno 2 e che siano in numero pari */
    if ((argc < 3) || ((argc - 1) % 2))
    {
        printf("Errore nel numero di parametri: ho bisogno di N nomi assoluti di file e N numeri con N >= 1 e nel formato: nomefile numero. Argc = %d\n", argc);
        exit(1);
    }

    N = (argc - 1) / 2;         /* Salvo il numero di file passati */

    /* Controllo che tutti i numeri passati siano strettamente positivi */
    for (i = 0; i < N; i++)
    {
        if((argv[(i + 1) * 2][0] == '-') || (argv[(i*2) + 1][0] == '0'))
        {
            printf("Errore nel passaggio dei parametri: %s non e' un numero strettamente positivo\n", argv[(i + 1) * 2]);
            exit(2);
        }
    }

    /* Alloco l'array di N pipe */
    piped = (pipe_t *) malloc(N * sizeof(pipe_t));
    if(piped == NULL)           /* Controllo che la malloc abbia avuto successo */
    {
        printf("Errore nella malloc\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if((pipe(piped[i])) < 0)   /* Controllo che la creazione della pipe abbia avuto successo */
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(4);
        }
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        if((pid = fork()) < 0)  /* Controllo che fork abbia avuto successo */
        {
            printf("Errore nella fork del figlio di indice i = %d\n", i);
            exit(5);
        }
        if(pid == 0)
        {
            /* Processo figlio */
            /* Procedo a chiudere tutte le pipe di cui non ho bisogno */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if (i != j)
                {
                    close(piped[j][1]);
                }
            }
            
            /* Creo la pipe per il processo nipote */
            if((pipe(p)) < 0)
            {
                printf("Errore nella creazione della pipe per il nipote\n");
                exit(-1);
            }

            /* Creo il processo nipote */
            if((pid = fork()) < 0)
            {
                printf("Errore nella fork del nipote\n");
                exit(-1);
            }
            if(pid == 0)
            {
                /* Processo nipote */
                /* Chiudo la pipe tra padre e figlio */
                close(piped[i][1]);

                /* Inizializzo il seme per la generazione di numeri random */
                srand(time(NULL));

                /* Salvo il numero di righe del file */
                X = atoi(argv[(i*2) + 2]);
                /* Genero un numero casuale di righe */
                r = mia_random(X);
                /* Costruisco la stringa di opzione per l'head */
                sprintf(opzione, "-%d", r);

                /* Chiudo lo standard output e lo sostituisco con la pipe in scrittura */
                close(1);
                dup(p[1]);

                /* Chiudo entrambi i lati della pipe */
                close(p[0]);
                close(p[1]);

                execlp("head", "head", opzione, argv[(i*2) + 1], (char *)0);

                /* Non dovremmo mai arrivare quì */
                perror("Errore nell'esecuzione della head");
                exit(-1);
            }

            /* Processo figlio */
            /* Chiudo in scrittura la pipe tra figlio e nipote */
            close(p[1]);
            
            /* Leggo dalla pipe fino a che ci sono caratteri inviati dal nipote con la head */
            j = 0;
            nrLinee = 0;
            S.pid_nipote = pid;
            while (read(p[0], &(S.linea[j]), 1))
            {
                if(S.linea[j] == '\n')      /* Caso in cui arrivo alla fine della riga */
                {
                    nrLinee++;
                    j++;
                    S.linea[j] = '\0';
                    S.n_linea = nrLinee;
                    write(piped[i][1], &S, sizeof(S));
                    j = 0;
                }
                else                        /* Caso in cui devo continuare a leggere caratteri dalla stessa linea */
                {
                    j++;
                }
            }

            /* Il filgio aspetta il nipote */
            ritorno = -1;            
            if((pid = wait(&status)) < 0)   /* Controllo che la wait vada a buon fine */
            {
                printf("Errore nella wait\n");
            }
            else if((status & 0xFF) != 0)
            {
                printf("Nipote con PID: %d terminato in modo anomalo\n", pid);
            }
            else
            {
                ritorno = nrLinee;
            }

            exit(ritorno);
        }
    }

    /* Processo padre */
    /* Chiudo tutte le pipe di cui non ho bisogno */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }

    finito = 0;        /* Setto a 0 la variabile finito per far partire il ciclo */
    while (!finito)
    {
        finito = 1;
        for (i = 0; i < N; i++)
        {
            nr = read(piped[i][0], &S, sizeof(S));
            
            if(nr != 0) /* Controllo che si sia letto qulache carattere */
            {
                finito = 0;
                 printf("Il nipote con pid %d ha letto dal file %s nella riga %d questa linea:\n%s",  S.pid_nipote, argv[(i*2)+1], S.n_linea, S.linea);
            } 
        }
    }
    
    /* Aspetto i processi figli */
    for (i = 0; i < N; i++)
    {
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(6);
        }
        if((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d e' terminato con valore %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
        
    
    exit(0);
}