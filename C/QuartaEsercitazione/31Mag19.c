#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco la struct */
typedef struct {
    int pidNipote;
    int len;
    char string[250];
} strut;

/* Definisco il tipo pipe_t */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di parametri passati */
    int h, i;                       /* Indici */
    int pid;                        /* Per fork */
    int nr;                         /* Parametri letti con la read */
    pipe_t *piped;                  /* Array di pipe di comunicazione tra padre e figli */
    int p[2];                       /* Pipe di comunicazione tra figlio e nipote */
    strut s;                        /* Struct che contiene tutti i dati */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di parametri passati */
    N = argc - 1;

    /* Faccio una malloc per l'array di pipe */
    if ((piped = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc\n");
        exit(2);
    }
    
    /* Creo le N pipe */
    for (h = 0; h < N; h++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(piped[h]) < 0)
        {
            printf("Errore nella creazione della pipe di indice h = %d\n", h);
            exit(3);
        }
        
    }
    
    /* Creo gli N processi figli */
    for (h = 0; h < N; h++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice h = %d\n", h);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe inutilizzate */
            for (i = 0; i < N; i++)
            {
                close(piped[i][0]);
                if (i != h)
                {
                    close(piped[i][1]);
                }
                
            }
            
            /* Creo la pipe di comunicazione tra figlio e nipote */
            if (pipe(p) < 0)
            {
                printf("Errore nella creazione della pipe tra figlio e nipote\n");
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
                /* Chiudo la pipe tra padre e figlio */
                close(piped[h][1]);
                
                /* Redireziono lo standard output su p[1] */
                close(1);
                dup(p[1]);

                /* Chiudo le pipe tra figlio e nipote */
                close(p[0]);
                close(p[1]);

                /* Eseguo il comando sort */
                execlp("sort", "sort", "-f", argv[h + 1], (char *)0);

                /* Non si dovrebbe arrivare quì */
                perror("Problemi di esecuzione del comando sort da parte del nipote");
                exit(-1);
            }
            
            /* Processo figlio */
            /* Chiudo la pipe tra filgio e nipote non utilizzata */
            close(p[1]);

            /* Salvo il pid del nipote nella struct */
            s.pidNipote = pid;
            
            /* Inizializzo i a 0 */
            i = 0;

            /* Leggo dalla pipe tra figlio e nipote un carattere alla volta */
            while (read(p[0], &(s.string[i]), 1))
            {
                /* Controllo se sono arrivato alla fine della linea */
                if (s.string[i] == '\n')
                {
                    s.len = i + 1;
                    break;
                }
                else
                {
                    i++;
                }
            }

            /* Il filgio comunica al padre */
            write(piped[h][1], &s, sizeof(s));
            
            /* Setto ritorno a -1 di default */
            ritorno = -1;

            /* Il figlio aspetta il nipote */
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
                ritorno = (int)((status >> 8) & 0xFF);
            }

            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Il padre chiude le pipe non utilizzate */
    for (h = 0; h < N; h++)
    {
        close(piped[h][1]);
    }
    
    /* Il padre recupera informazioni dai figli */
    for (h = 0; h < N; h++)
    {
        /* Leggo la struttura scritta dal figlio */
        nr = read(piped[h][0], &s, sizeof(s));
        if (nr != 0)
        {
            /* Aggiungo il terminatore alla stringa */
            s.string[s.len - 1] = '\0';
            printf("Il nipote con PID: %d ha letto dal file %s la stringa %s di lunghezza %d (compreso il terminatore)\n", s.pidNipote, argv[h + 1], s.string, s.len); 
        }
        
    }
    
    /* Il padre aspetta i figli */
    for (h = 0; h < N; h++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}