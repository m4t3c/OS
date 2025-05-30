#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

void handler(int signo)
{
    printf("DEBUG-Sono il padre %d e ho ricevuto il segnale %d\n", getpid(), signo);
}

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                              /* Numero di file passati per i processi figli */
    int *pid;                           /* Array di pid per la fork */
    int *confronto;                     /* Array per il confronto che stabilisce se continuare o meno il processo */
    pipe_t *pipe_pf;                    /* Array di pipe di comunicazione padre figlio */
    pipe_t *pipe_fp;                    /* Array di Pipe di comunicazione figlio padre */
    int i, j;                           /* Indici per i cicli */
    int fd;                             /* File descriptor per il file */
    char token = 'v';                   /* Variabile che contiene il carattere inviato dalla pipe padre-figlio */
    char c;                             /* Variabile che contiene il carattere letto da file nel processo figlio */
    char ch;                            /* Variabile che contiene il carattere letto da file nel processo padre */
    int  pidFiglio, status, ritorno;    /* Per wait */
    /* ------------------------------ */
    
    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno tre parametri (nomi di file) ma argc = %d\n", argc);
        exit(1);
    }

    /* Assegno ad N il numero di parametri passati escluso l'ultimo */
    N = argc - 2;

    /* Installo il gestore handler per il segnale SIGPIPE */
    signal(SIGPIPE, handler);

    /* Alloco memoria per l'array di pid */
    pid = (int *)malloc(N * sizeof(int));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (pid == NULL)
    {
        printf("Errore nalla malloc dell'array di pid\n");
        exit(2);
    }

    /* Alloco memoria per l'array confronto */
    confronto = (int *)malloc(N * sizeof(int));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (confronto == NULL)
    {
        printf("Errore nella malloc dell'array confronto\n");
        exit(3);
    }

    /* Setto l'array di confronto ad 1 */
    for (i = 0; i < N; i++)
    {
        confronto[i] = 1;
    }

    /* Alloco memoria per l'array di pipe padre figlio */
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la sua allocazione sia andata a buon fine */
    if (pipe_pf == NULL)
    {
        printf("Errore nell'allocazione dell'array di pipe padre-figlio\n");
        exit(4);
    }
    /*Alloco memoria cper l'array di pipe figlio padre */
    pipe_fp = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la sua allocazione sia andata a buon fine */
    if (pipe_fp == NULL)
    {
        printf("Errore nell'allocazione dell'array di pipe figlio-padre\n");
        exit(5);
    }

    /* Creo le pipe padre figlio e figlio padre */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipe_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe padre-figlio di indice i = %d\n", i);
            exit(6);
        }

        if (pipe(pipe_fp[i]) < 0)
        {
            printf("Errore nella creazione della pipe figlio-padre di indice i = %d\n", i);
            exit(7);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork abbia successo */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella creazione del figlio di indice i = %d\n", i);
            exit(8);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) per riportare un errore */
            /* Chiudo tutte le pipe che non mi servono */
            for (j = 0; j < N; j++)
            {
                close(pipe_fp[j][0]);
                close(pipe_pf[j][1]);
                if (i != j)
                {
                    close(pipe_fp[j][1]);
                    close(pipe_pf[j][0]);
                }
            }

            /* Apro in lettura il file */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }

            /* Itero un ciclo finchè posso leggere dalla pipe padre-figlio */
            while (read(pipe_pf[i][0], &token, 1))
            {
                /* Se il token contiene t allora il processo deve terminare */
                if (token == 't')
                {
                    break;
                }

                read(fd, &c, 1);
                write(pipe_fp[i][1], &c, 1);
            }

            exit(0);
        }
    }

    /* Codice padre */
    /* Chiudo le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipe_fp[i][1]);
        close(pipe_pf[i][0]);
    }

    /* Apro il file in lettura */
    if ((fd = open(argv[argc - 1], O_RDONLY)) < 0)
    {
        printf("Impossibile aprire in lettura il file %s\n", argv[argc - 1]);
        exit(9);
    }

    /* Itero un ciclo che va avanti finchè leggo caratteri */
    while (read(fd, &ch, 1))
    {
        /* Confronto il carattere letto con tutti quelli inviati dai figli */
        for (i = 0; i < N; i++)
        {
            /* Nel caso il processo sia ancora attivo controllo se i caratteri siano uguali */
            if (confronto[i])
            {
                write(pipe_pf[i][1], &token, 1);
                read(pipe_fp[i][0], &c, 1);
                if (c != ch)
                {
                    confronto[i] = 0;
                }
            }
        }
    }

    /* Itero un ciclo che termina forzatamente i processi che hanno letto un carattere diverso mentre termina quando il file è finito */
    for (i = 0; i < N; i++)
    {
        if (!confronto[i])
        {
            /* Controllo che la kill non fallisca */
            if ((kill(pid[i], SIGKILL)) == -1)
            {
                printf("Processo filgio con pid = %d non esiste quindi gia' terminato\n", pid[i]);
            }
            
        }
        else
        {
            token = 't';
            write(pipe_pf[i][1], &token, 1);
        }
    }

    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(10);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il figlio con PID: %d e' terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            if (ritorno == 255)
            {
                printf("Il figlio con PID: %d ha ritornato 255 quindi ci sono stati problemi\n", pidFiglio);
            }
            else
            {
                for (j = 0; j < N; j++)
                {
                    if(pid[j] == pidFiglio)
                    {
                        printf("Il figlio con PID: %d e di indice %d ha ritornato il valore %d. Quindi il file %s e' uguale al file %s\n", pidFiglio, j, ritorno, argv[j + 1], argv[argc - 1]);
                    }
                }
                
            }
        }
    }

    exit(0);
}