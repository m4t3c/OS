#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di due int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                              /* Numero di file passati come parametri */
    int C;                              /* Numero passato come ultimo parametro */
    int pid;                            /* Per fork */ 
    pipe_t *pipes;                      /* Array di N pipe */
    int i;                              /* Indice processi figli */
    int j;                              /* Indice per il ciclo */
    char *b;                            /* Buffer per la read */
    char *FCreato;                      /* Stringa che conterrà il nome del file da creare */
    int fd;                             /* Per open */
    int nr, nw;                         /* Per write e read */
    int filecreato;                     /* Variabile che contiene il file descriptor del file creato */
    int nroTot;                         /* Numero totale di blocchi C nel file */
    int nro;                            /* Numero di blocchi che il processo deve leggere */
    int pidFiglio, status, ritorno;     /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile, ..., numdisp) ma argc = %d\n", argc);
        exit(1);
    }

    /* Controllo che l'ultimo parametro sia un numero positivo dispari */
    C = atoi(argv[argc - 1]);
    if (C < 0 || (C % 2) == 0)
    {
        printf("Errore nel passaggio dei parametri: %d non e' strettamente positivo oppure non e' dispari\n", C);
        exit(2);
    }
    
    /* Allocazione dinamica del buffer b */
    b = (char *)malloc(C);
    /* Controllo che la malloc sia andata a buon fine */
    if (b == NULL)
    {
        printf("Errore nella malloc\n");
        exit(3);
    }
    
    /* Salvo il numero di file passati nella variabile N */
    N = argc - 2;

    /* Alloco spazio per le N pipe */
    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc\n");
        exit(4);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione vada a buon fine */
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(5);
        }
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < 2 * N; i++)
    {
        /* Controllo che la fork abbia successo */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* In caso di errore si sceglie di ritornare -1 (255 senza segno) */
            if (i < N)
            {
                /* Chiudo tutte le pipe in lettura e chiudo in scrittura solo quelle diverse dalla pipe del processo corrente */
                for (j = 0; j < N; j++)
                {
                    close(pipes[j][0]);
                    if (i != j)
                    {
                        close(pipes[j][1]);
                    }
                }
                
                /* Apro in lettura il file associato */
                if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file %s da parte del figlio di indice i = %d\n", argv[i + 1], i);
                    exit(-1);
                }
                
                /* Calcolo la lunghezza totale del file in termini di blocchi C */
                nroTot = lseek(fd, 0L, 2) / C;
                /* Calcolo il numero di blocchi C che il primo processo della coppia deve leggere */
                nro = nroTot / 2;
                
                /* Torno all'inizio del file */
                lseek(fd, 0L, SEEK_SET);

                /* Leggo nro blocchi C dal file */
                for (j = 0; j < nro; j++)
                {
                    read(fd, &b, C);
                    /* Scrivo sulla pipe il blocco letto */
                    nw = write(pipes[i][1], &b, C);
                    /* Controllo che siano stati scritti un numero corretto di byte */
                    if (nw != C)
                    {
                        printf("Errore in scrittura su pipe %d\n", i);
                        exit(-1);
                    }
                }
            }
            else
            {
                /* Alloco memoria per la stringa che conterra' il nome del file */
                FCreato = (char *)malloc((strlen(argv[i - N + 1])) + 11);
                /* Controllo che la malloc sia andata a buon fine */
                if (FCreato == NULL)
                {
                    printf("Errore nella malloc\n");
                    exit(-1);
                }
                
                /* Creo la stringa del nome del file */
                strcpy(FCreato, argv[i - N + 1]);
                strcat(FCreato, ".mescolato");

                /* Creo il nuovo file */
                if ((filecreato = creat(FCreato, PERM)) < 0)
                {
                    printf("Errore nella creazione del file %s\n", FCreato);
                    exit(-1);
                }

                /* Chiudo tutte le pipe che non mi servono */
                for (j = 0; j < N; j++)
                {
                    close(pipes[j][1]);
                    if ((i - N) != j)
                    {
                        close(pipes[j][0]);
                    }
                }
                
                /* Apro il file associato alla coppia di processi figli */
                if ((fd = open(argv[i - N + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file %s\n", argv[i - N + 1]);
                    exit(-1);
                }
                
                /* Calcolo la lunghezza totale del file in termini di blocchi C */
                nroTot = lseek(fd, 0L, 2) / C;
                /* Calcolo il numero di blocchi C che il primo processo della coppia deve leggere */
                nro = nroTot / 2;

                /* Porto a metà il puntatore del file */
                lseek(fd, (long)nro * C, 0);
                
                /* Leggo i restanti blocchi da leggere */
                for (j = 0; j < nro; j++)
                {
                    read(fd, &b, C);

                    /* Scrivo sul file creato il blocco appena letto */
                    write(filecreato, &b, C);
                    
                    /* Leggo il blocco letto dal primo processo figlio scritto sulla pipe della coppia di processi */
                    nr = read(pipes[i - N][0], &b, C);
                    if (nr != C)
                    {
                        printf("Errore di lettura da pipe %d\n", i - N);
                        exit(-1);
                    }
                    
                    /* Scrivo il blocco sul file creato */
                    write(filecreato, &b, C);
                }
            }

            exit(nro);
        }
    }

    /* Codice del padre */
    /* Chiudo tutte le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Attendo i figli */
    for (i = 0; i < N*2; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID = %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }

    exit(0);
}