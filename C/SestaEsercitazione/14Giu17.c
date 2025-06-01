#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di due int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{   
    /* ------ Variabili locali ------ */
    int pid;                            /* Per fork */
    int N;                              /* Numero di file passati nonche' numero di processi figli da creare */
    char Cx;                            /* Carattere passato come ultimo parametro */
    pipe_t *pipe_pf;                    /* Pipe di comunicazione tra processo padre e figlio */
    pipe_t *pipe_fp;                    /* Pipe di comunicazione tra processo figlio e padre */
    int i, j;                           /* Indici per i cicli */
    int fd;                             /* File descriptor per la open */
    char c;                             /* Buffer per il carattere letto da file */
    char cx;                            /* Carattere letto dalla pipe del padre */
    char scarto;                        /* Variabile per eliminare l'invio */
    long int pos;                       /* Posizione del carattere cercato */
    int ritorno;                        /* Valore di ritorno per processo figlio e wait */
    int finito;                         /* Variabile di controllo che indica se i processi figli hanno terminato la lettura del file */
    int nr;                             /* Variabile di controllo per read */
    int status;                         /* Per wait */
    /* -------------------------------*/
    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, ..., carattere) ma argc = %d\n", argc);
        exit(1);
    }

    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo crattere\n", argv[argc - 1]);
        exit(2);
    }
    /* Salvo l'ultimo parametro nella variabile Cx */
    Cx = argv[argc - 1][0];
    
    /* Alloco memoria per le due pipe tra padre e figlio e figlio padre */
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipe_fp = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if ((pipe_pf == NULL) || (pipe_fp == NULL))
    {
        printf("Errore nella malloc per le pipe\n");
        exit(3);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la i-esima pipe tra padre e figlio sia creata correttamente */
        if (pipe(pipe_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra padre e figlio di indice i = %d\n", i);
            exit(4);
        }

        /* Controllo che la i-esima pipe tra filgio e padre sia creata correttamente */
        if (pipe(pipe_fp[i]) < 0)
        {
            printf("Errore nella creazione della pipe tra figlio e padre di indice i = %d\n", i);
            exit(5);
        }
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork abbia successo */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) per segnalare errore */
            /* Chiudo le pipe che non servono al processo figlio */
            for (j = 0; j < N; j++)
            {
                close(pipe_fp[j][0]);
                close(pipe_pf[j][1]);
                if (j != i)
                {
                    close(pipe_fp[j][1]);
                    close(pipe_pf[j][0]);
                }
            }
            
            /* Apro in lettura-scrittura il file passato come i+1-esimo parametro */
            if ((fd = open(argv[i + 1], O_RDWR)) < 0)
            {
                printf("Errore nell'apertura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Itero un ciclo che legge un carattere alla volta da file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == Cx)
                {
                    pos = lseek(fd, 0L, SEEK_CUR);
                    write(pipe_fp[i][1], &pos, sizeof(pos));
                    read(pipe_pf[i][0], &cx, 1);
                    if (cx != '\n')
                    {
                        lseek(fd, -1L, SEEK_CUR);
                        write(fd, &cx, 1);
                        ritorno++;
                    }
                }   
            }

            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe che non mi servono */
    for (i = 0; i < N; i++)
    {
        close(pipe_pf[i][0]);
        close(pipe_fp[i][1]);
    }
    
    finito = 0;
    /* Itero un ciclo per recuperare dal figlio le informazioni */
    while (!finito)
    {
        finito = 1;
        for (i = 0; i < N; i++)
        {
            /* Leggo la posizione inviata dall'i-esimo figlio */
            nr = read(pipe_fp[i][0], &pos, sizeof(pos));
            /* Controllo che sia stato letto qualcosa dalla pipe */
            if (nr != 0)
            {
                finito = 0;
                printf("Il processo figlio di indice i = %d ha trovato il carattere %c nel file %s nella posizione %ld.\nSe vuoi sostituirlo indica il carattere con cui vorresti effettuare la sostituzione\n", i, Cx, argv[i + 1], pos);
                read(0, &cx, 1);
                if (cx != '\n')
                {
                    /* Eliminiamo il carattere invio */
                    read(0, &scarto, 1);
                }
                
                write(pipe_pf[i][1], &cx, 1);
            }
        }
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);

}