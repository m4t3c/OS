#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

/* definizione del TIPO pipe_t come array di 2 interi */
typedef int pipe_t[2];

void handler(int signo)
{
    printf("DEBUG-Sono il padre %d e ho ricevuto il segnale %d\n", getpid(), signo);
}

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di parametri passati */
    int fd;                         /* Per open */
    int *pid;                       /* Array di pid */
    int *confronto;                 /* Array per sapere se mandare l'indicazione v o t ai processi figli */
    pipe_t *pipes_pf;               /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t *pipes_fp;               /* Array di pipe di comunicazione tra figlio e nipote */
    int i, j;                       /* Indici per i cicli */
    char c, ch;                     /* Caratteri letti da file e da pipe */
    char token = 'v';               /* Carattere da inviare ai figli per dirgli di terminare o continuare */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel passaggio dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di parametri nella variabile N */
    N = argc - 2;

    /* il padre installa il gestore handler per il segnale SIGPIPE */
    signal(SIGPIPE, handler);

    /* Alloco memoria per l'array di pid */
    if ((pid = (int *)malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella malloc per l'array di pid\n");
        exit(2);
    }

    /* Alloco memoria per l'array confronto */
    if ((confronto = (int *)malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella malloc per l'array del confronto\n");
        exit(3);
    }

    /* Inizializzo il vettore confronto con tutti 1 */
    for (i = 0; i < N; i++)
    {
        confronto[i] = 1;
    }

    /* Alloco memoria per l'array di pipe tra padre e figlio */
    if ((pipes_pf = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pid tra padre e filgio\n");
        exit(4);
    }

    /* Alloco memoria per l'array di pipe tra figlio e padre */
    if ((pipes_fp = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pid tra figlio e padre\n");
        exit(5);
    }

    /* Creo le pipe tra padre e figlio e tra figlio e padre */
    for (i = 0; i < N; i++)
    {
        /* Controllo che l'iesima pipe tra padre e figlio sia creata correttamente */
        if ((pipe(pipes_pf[i])) < 0)
        {
            printf("Errore nella creazione della pipe tra padre e figlio di indice i = %d\n", i);
            exit(6);
        }

        /* Controllo che l'iesima pipe tra figlio e padre sia creata correttamente */
        if ((pipe(pipes_fp[i])) < 0)
        {
            printf("Errore nella creazione della pipe tra figlio e padre di indice i = %d\n", i);
            exit(7);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che l'iesimo processo figlio sia creato correttamente */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella fork per il processo figlio di indice i = %d\n", i);
            exit(8);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (j = 0; j < N; j++)
            {
                close(pipes_fp[j][0]);
                close(pipes_pf[j][1]);
                if (j != i)
                {
                    close(pipes_fp[j][1]);
                    close(pipes_pf[j][0]);
                }
            }

            /* Apro il file in lettura */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[i + 1]);
                exit(-1);
            }

            /* Itero un ciclo che va avanti finchè riceve dal padre l'ok per proseguire */
            while (read(pipes_pf[i][0], &token, 1))
            {
                /* Se il carattere ricevuto è t devo terminare il ciclo */
                if (token == 't')
                {
                    break;
                }

                /* In caso contrario leggo un singolo carattere dal file e lo mando al padre */
                read(fd, &c, 1);
                write(pipes_fp[i][1], &c, 1);
            }

            exit(0);
        }
    }

    /* Processo padre */
    /* Chiudo le pipe inutilizzate */
    for (i = 0; i < N; i++)
    {
        close(pipes_fp[i][1]);
        close(pipes_pf[i][0]);
    }

    /* Apro in lettura il file */
    if ((fd = open(argv[argc - 1], O_RDONLY)) < 0)
    {
        printf("Errore nella open del file %s\n", argv[argc - 1]);
        exit(9);
    }

    /* Itero un ciclo che legge carattere per carattere il file */
    while (read(fd, &ch, 1))
    {
        /* Faccio il confronto per ogni figlio che ha ancora attiva la lettura */
        for (i = 0; i < N; i++)
        {
            if (confronto[i])
            {
                /* Invio al figlio il token */
                write(pipes_pf[i][1], &token, 1);

                /* Recupero il carattere dalla pipe tra figlio e padre */
                read(pipes_fp[i][0], &c, 1);

                /* Controllo se il file letto dal padre e letto dal figlio sono uguali */
                if (ch != c)
                {
                    confronto[i] = 0;
                }
            }
        }
    }

    /* Una volta terminata la lettura da file termino i processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo se l'iesimo figlio ha ancora il confronto attivo o meno */
        if (!confronto[i])
        {
            /* Controllo che la kill non fallisca */
            if ((kill(pid[i], SIGKILL)) == -1)
            {
                printf("Figlio con PID: %d non esiste quindi è già terminato\n", pid[i]);
            }
        }
        else
        {
            /* Inizializzo token a 't' */
            token = 't';
            write(pipes_pf[i][1], &token, 1);
        }
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
            printf("Il processo figlio con PID: %d è terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            if (ritorno == 255)
            {
                printf("Il figlio con pid=%d ha ritornato il valore %d e quindi ci sono stati problemi\n", pidFiglio, ritorno);
            }
            else
            {
                /* se un figlio termina normalmente vuol dire che non e' stato ucciso dal SIGKILL: ATTENZIONE CHE DOBBIAMO RECUPERARE L'INDICE DI CREAZIONE USANDO L'ARRAY DI pid! */
                for (j = 0; j < N; j++)
                {
                    if (pid[j] == pidFiglio)
                        printf("Il figlio con pid=%d e indice %d ha ritornato il valore %d e quindi ha verificato che il file %s e' uguale al file %s\n", pidFiglio, j, ritorno, argv[j + 1], argv[argc - 1]);
                }
            }
        }
    }

    exit(0);
}
