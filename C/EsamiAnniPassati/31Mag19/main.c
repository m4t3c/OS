#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Definisco la struct Strut con i campi richiesti dal testo */
typedef struct 
{
    int pidNipote;      /* Campo c1 del testo */
    int lenLinea;       /* Campo c2 del testo */
    char linea[250];    /* Campo c3 del testo */
} Strut;

int main(int argc, char **argv)
{
    
    /* ------ Variabili locali ------ */
    int N;                          /* Numero di parametri/processi figli */
    int pid;                        /* Per fork e wait */
    pipe_t *pipes;                  /* Array di pipe tra padre e figli */
    pipe_t p;                       /* Pipe di comunicazione tra figlio e nipote */
    int i, j;                       /* Indici per i cicli */
    Strut S;                        /* Struct che contiene i dati richiesti */
    int nr;                         /* Per read */
    int ritorno, status;            /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, nomefile3, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Assegno ad N il numero di parametri passati */
    N = argc - 1;

    /* Alloco spazio per l'array di pipe */
    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc\n");
        exit(2);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esima pipe vada a buon fine */
        if (pipe(pipes[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare in caso di errore -1 (255 senza segno) */
            /* Chiudo le pipe che non servono */
            for (j = 0; j < N; j++)
            {
                close(pipes[j][0]);
                if (j != i)
                {
                    close(pipes[j][1]);
                }
            }
            
            /* Creo la pipe tra figlio e nipote */
            if (pipe(p) < 0)
            {
                printf("Errore nella creazione della pipe tra figlio e nipote di indice i = %d\n", i);
                exit(-1);
            }
            
            /* Creo il processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella crezione del processo nipote di indice i = %d\n", i);
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Si sceglie di ritornare in caso di errore -1 (255 senza segno) */
                /* Chiudo le pipe che non servono */
                close(pipes[i][1]);

                /* Simulo il piping dei comandi */
                close(1);
                dup(p[1]);

                /* Chiudo in entrambi i lati la pipe tra figlio e nipote */
                close(p[0]);
                close(p[1]);

                /* Eseguo il comando sort con l'opzione di ignorare la differenza tra maiuscole e minuscole */
                execlp("sort", "sort", "-f", argv[i + 1], (char *)0);

                /* Non si dovrebbe mai arrivare qui' */
                perror("Errore nell'esecuzione del comando sort -f");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Chiudo la pipe tra figlio e nipote in scrittura */
            close(p[1]);
            /* Inizializzo la struct S */
            S.pidNipote = pid;
            
            /* Inizializzo j a 0 che fungera' come contatore */
            j = 0;
            /* Itero un ciclo che legge la prima riga del file */
            while (read(p[0], &(S.linea[j]), 1))
            {
                if (S.linea[j] == '\n')
                {
                    S.lenLinea = j + 1;
                    break;
                }
                else
                {
                    ++j;
                }
            }
            
            /* Invio al padre la struct S */
            write(pipes[i][1], &S, sizeof(S));
            
            /* Il figlio aspetta il nipote */
            ritorno = -1;
            if ((pid = wait(&status)) < 0)
            {
                printf("Errore nella wait\n");
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d terminato in modo anomalo\n", pid);
            }
            else
            {
                ritorno = S.lenLinea - 1;
            }

            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo le pipe che non servono */
    for (i = 0; i < N; i++)
    {
        close(pipes[i][1]);
    }
    
    /* Il padre recupera le informazioni dai figli */
    for (i = 0; i < N; i++)
    {
        nr = read(pipes[i][0], &S, sizeof(S));
        if (nr != 0)
        {
            S.linea[S.lenLinea] = '\0';
            printf("Il processo nipte con PID: %d ha trovato nel file %s la seguente linea di %d caratteri: %s", S.pidNipote, argv[i + 1], S.lenLinea, S.linea);
        }
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}