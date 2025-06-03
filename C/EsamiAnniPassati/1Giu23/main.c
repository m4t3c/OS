#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di due int */
typedef int pipe_t[2];

/* Definisco la struct Strut */
typedef struct 
{
    int pid_nipote;             /* Campo c1 del testo */
    char linea_letta[250];      /* Campo c2 del testo */
    int lun_linea;              /* Campo c3 del testo */
} Strut;

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file passati */
    int pid;                        /* Per fork */
    pipe_t *piped;                  /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t p;                       /* Pipe tra figlio e nipote */
    int n, i;                       /* Indici dei cicli */
    int nr;                         /* Per la read */
    char buffer[250];               /* Buffer per la linea letta */
    Strut S;                        /* Struct che contiene i dati richiesti dal testo */
    int status, ritorno;            /* Per wait */
    /* ------------------------------ */
    
    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, nomefile2, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri nella variabile N */
    N = argc - 1;

    /* Alloco memoria per l'array di pipe */
    piped = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (piped == NULL)
    {
        printf("Errore nella malloc\n");
        exit(2);
    }
    
    /* Creo le N pipe tra padre e figlio */
    for (n = 0; n < N; n++)
    {
        if (pipe(piped[n]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", n);
            exit(3);
        }
    }
    
    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* In caso di errore si sceglie di ritornare -1 (255 senza segno) */
            /* Chiudo le pipe inutilizzate */
            for (i = 0; i < N; i++)
            {
                close(piped[i][0]);
                if (i != n)
                {
                    close(piped[i][1]);
                }
            }
            
            /* Creo la pipe tra figlio e nipte */
            if (pipe(p) < 0)
            {
                printf("Errore nella creazione della pipe tra figlio e nipote di indice n = %d\n", n);
                exit(-1);
            }
            
            /* Creo il processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote di indice n = %d\n", n);
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* In caso di errore si sceglie di ritornare -1 (255 senza segno) */
                /* Chiudo la pipe del processo figlio inutilizzata */
                close(piped[n][1]);

                /* Simulo il piping dei comandi */
                close(1);
                dup(p[1]);

                /* Chiudo le pipe inutilizzate */
                close(p[0]);
                close(p[1]);

                /* Eseguo il comando rev */
                execlp("rev", "rev", argv[n + 1], (char *)0);
                /* Non si dovrebbe mai arrivare qui' */
                perror("Errore nella esecuzione del comando rev\n");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Chiudo la pipe non usata */
            close(p[1]);

            /* Scrivo sulla struct S il pipe del nipote */
            S.pid_nipote = pid;

            /* Inizializzo i a 0 */
            i = 0;

            /* Leggo caratteri finche' non arrivo all'ultima riga */
            while (read(p[0], &(buffer[i]), 1))
            {
                /* Controllo se siamo arrivati alla fine di una riga */
                if (buffer[i] == '\n')
                { 
                    S.lun_linea = i + 1;
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
            
            /* Essendo giunto all'ultima linea la salvo nella struct */
            for (i = 0; i < S.lun_linea; i++)
            {
                S.linea_letta[i] = buffer[i];
            }
            
            /* Scrivo sulla pipe la struct */
            write(piped[n][1], &S, sizeof(S));

            /* Il filgio aspetta il nipote */
            if ((pid = wait(&status)) < 0)
            {
                printf("Errore nella wait del nipote\n");
                exit(-1);
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d terminato in modo anomalo\n", pid);
                exit(-1);
            }
            else
            {
                ritorno = S.lun_linea - 1;
            }

            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe che non mi servono */
    for (n = 0; n < N; n++)
    {
        close(piped[n][1]);
    }
    
    /* Recupero le informazioni dai figli */
    for (n = 0; n < N; n++)
    {
        nr = read(piped[n][0], &S, sizeof(S));
        if (nr != 0)
        {
            /* Aggiungo il terminatore */
            S.linea_letta[S.lun_linea] = '\0';
        }

        printf("Il nipote con PID: %d ha letto dal file %s questa linea '%s' lunga %d caratteri (compreso il terminatore)\n", S.pid_nipote, argv[n + 1], S.linea_letta, S.lun_linea);
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi)\n", pid, ritorno);
        }
    }
    
    exit(0);
}