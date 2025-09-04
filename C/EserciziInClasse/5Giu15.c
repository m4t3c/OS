#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int M;                          /* Numero di file passati come parametro */
    int pid;                        /* Per fork */
    pipe_t *pipes_pf;               /* Array di pipe di comunicazione tra padre e figlio */
    pipe_t p;                       /* Pipe di comunicazione tra figlio e nipote */
    int j, k;                       /* Indici per i cicli */
    char ch;                        /* Singolo carattere letto dal figlio dalla pipe figlio nipote */
    int l;                          /* Variabile che conta i caratteri letti */
    int lunghezza;                  /* Lunghezza dell'ultima riga del file */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo M con il numero di parametri passati */
    M = argc - 1;

    /* Alloco memoria per l'array di pipe */
    if ((pipes_pf = (pipe_t*)malloc(M * sizeof(pipes_pf))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(2);
    }
    
    /* Creo le M pipe */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(pipes_pf[j]) < 0)
        {
            printf("Errore nella creazione della pipe di indice j = %d\n", j);
            exit(3);
        }
        
    }
    
    /* Creo gli M processi figli */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            for (k = 0; k < M; k++)
            {
                close(pipes_pf[k][0]);
                if (k != j)
                {
                    close(pipes_pf[k][1]);
                }
                
            }
            
            /* Creo la pipe tra figlio e nipote */
            if (pipe(p) < 0)
            {
                printf("Erore nella creazione della pipe tra figlio e nipote\n");
                exit(-1);
            }
            
            /* Creo il processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote\n");
                close(-2);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Chiudo le pipe non necessarie */
                close(pipes_pf[j][1]);

                /* Simulo il piping dei comandi reindirizzando standard outpust sulla pipe */
                close(1);
                dup(p[1]);

                /* Chiudo le restanti pipe */
                close(p[0]);
                close(p[1]);

                /* Reindirizzo standard error su /dev/null */
                close(2);
                open("/dev/null", O_WRONLY);

                /* Eseguo il comando tail */
                execlp("tail", "tail", "-1", argv[j + 1], (char *)0);

                /* Non si dovrebbe arrivare quì */
                printf("Errore nell'esecuzione del comando tail\n");
                exit(-3);
            }
            
            /* Processo figlio */
            /* Chiudo le pipe non necessarie */
            close(p[1]);

            /* Inizializzo l a 0 */
            l = 0;

            /* Leggo un carattere alla volta la riga inviata dal processo figlio */
            while (read(p[0], &ch, 1))
            {
                l++;
            }
            
            /* Se l != 0 inizializzo lunghezza a l - 1 per escludere il terminatore */
            if (l != 0)
            {
                lunghezza = l - 1;
            }
            else
            {
                lunghezza = 0;
            }

            /* Invio lunghezza al padre */
            write(pipes_pf[j][1], &lunghezza, sizeof(lunghezza));

            /* Di default imposto ritorno a -1 */
            ritorno = -1;

            /* Il figlio aspetta i nipoti */
            if ((pidFiglio = wait(&status)) < 0)
            {
                printf("Errore nella wait del processo nipote\n");
                exit(-4);
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote con PID: %d ritornato in modo anomalo\n", pidFiglio);
            }
            else
            {
                ritorno = (int)((status >> 8) & 0xFF);
            }

            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (j = 0; j < M; j++)
    {
        close(pipes_pf[j][1]);
    }
    
    /* Recupero le informazioni dai figli */
    for (j = 0; j < M; j++)
    {
        read(pipes_pf[j][0], &lunghezza, sizeof(lunghezza));
        printf("Il processo figlio di indice %d ha comunicato il valore %d per il file %s\n", j, lunghezza, argv[j + 1]);
    }
    
    /* Il padre aspetta i figli */
    for (j = 0; j < M; j++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
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