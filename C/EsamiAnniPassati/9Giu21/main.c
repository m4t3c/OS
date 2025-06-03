#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file/processi figli */
    int fcreato;                    /* File descriptor del file creato su /tmp */
    int fd;                         /* File descriptor del file associato al processo figlio aperto in lettura */
    int pidPrimoFiglio;             /* Per la fork del primo figlio */
    int pid;                        /* Per fork e wait */
    int X;                          /* Numero di linee nei file */
    pipe_t p;                       /* Pipe per il primo figlio */
    pipe_t *piped;                  /* Array di pipe */
    int n, i;                       /* Indici dei cicli */
    char linea[250];                /* Buffer per leggere da pipe */
    int ritorno;                    /* Valore di ritorno dei processi figli e per wait */
    int valore;                     /* Lunghezza di ogni linea letta dal padre */
    int status;                     /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, nomefile2, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri passati nella variabile N */
    N = argc - 1;

    /* Creo il file fcreato */
    if ((fcreato = creat("/tmp/MatteoCoppa", PERM)) < 0)
    {
        printf("Errore nella creazione del file /tmp/MatteoCoppa\n");
        exit(2);
    }
    
    /* Creo la singola pipe per il primo processo figlio */
    if (pipe(p) < 0)
    {
        printf("Errore nella creazione della pipe del primo figlio\n");
        exit(3);
    }

    /* Creo il processo filgio che comunica il numero di righe al padre */
    if ((pidPrimoFiglio = fork()) < 0)
    {
        printf("Errore nella creazione del primo processo figlio\n");
        exit(4);
    }
    if (pidPrimoFiglio == 0)
    {
        /* Primo processo figlio */
        /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
        /* Simulo il piping dei comandi */
        close(0);
        if (open(argv[1], O_RDONLY))
        {
            printf("Errore nell'apertura del file %s\n", argv[1]);
            exit(-1);
        }
        
        close(1);
        dup(p[1]);
        /* Chiudo la pipe in lettura e scrittura */
        close(p[0]);
        close(p[1]);
        
        /* Eseguo il comando wc */
        execlp("wc", "wc", "-l", (char *)0);

        /* Non si dovrebbe mai arrivare qui' */
        perror("Errore nell'esecuzione del comando wc");
        exit(-1);
    }
    
    /* Processo padre */
    /* Chiudo la pipe in scrittura */
    close(p[1]);

    /* Il padre recupera l'informazione dal figlio */
    i = 0;
    while (read(p[0], &(linea[i]), 1))
    {
        ++i;
    }
    if (i != 0)
    {
        linea[i - 1] = '\0';
        X = atoi(linea);
    }
    else
    {
        printf("Il processo padre non ha ricevuto valori dal primo processo figlio\n");
        exit(5);
    }
    
    /* Chiudo anche il lato in lettura della prima pipe */
    close(p[0]);

    /* Alloco memoria per l'array di pipe */
    piped = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (piped == NULL)
    {
        printf("Errore nella malloc\n");
        exit(6);
    }
    
    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        if (pipe(piped[n]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", n);
            exit(7);
        }
    }

    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(8);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo le pipe non necessarie */
            for (i = 0; i < N; i++)
            {
                close(piped[i][0]);
                if (i != n)
                {
                    close(piped[i][1]);
                }
            }
            
            /* Apro il file associato */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Setto a zero i che verra' usato come indice */
            i = 0;

            /* Scorro tutti i caratteri del file */
            while (read(fd, &(linea[i]), 1))
            {
                if (linea[i] == '\n')
                {
                    ++i;
                    write(piped[n][1], &i, sizeof(i));
                    write(piped[n][1], linea, i);
                    ritorno = i;
                    i = 0;
                }
                else
                {
                    ++i;
                }
            }
            
            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (n = 0; n < N; n++)
    {
        close(piped[n][1]);
    }
    
    /* Il padre recupera le informazioni di tutte le righe dai processi figli */
    for (i = 0; i < X; i++)
    {
        for (n = 0; n < N; n++)
        {
            read(piped[n][0], &valore, sizeof(valore));
            read(piped[n][0], &linea, valore);
            write(fcreato, linea, valore);
        }
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N + 1; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(9);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}