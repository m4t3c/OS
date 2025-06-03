#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];      /* Definisco il tipo pipe_t come array di 2 interi */

typedef struct 
{
    int pid_nipote;
    int lunghezza_linea;
    char linea[250];
} Struct;


int main(int argc, char **argv)
{
    int pid;                            /* Variabile per salvare la fork */
    int N;                              /* Variabile per salvare il numero di parametri passati */
    int i, j;                           /* Indici per i cicli */
    pipe_t *piped;                      /* Array di pipe di comunicazione tra processo figlio e padre */
    pipe_t p;                           /* Singola pipe di comunicazione tra processo nipote e figlio */
    Struct S;                           /* Struct usata dai figli e dal padre */
    int nr;                             /* Variabile per salvare il valore di ritorno */
    int ritorno, status;                /* Per wait */
    
    if (argc < 4)                       /* Controllo che siano passati almeno 3 parametri */
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomi assoluti di file) ma argc = %d\n", argc);
        exit(1);
    }

    N = argc - 1;

    /* Alloco spazio per N pipe nell'array piped */
    piped = (pipe_t *) malloc(N * sizeof(pipe_t));
    if(piped == NULL)                   /* Controllo che l'allocazione sia andata a buon fine */
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(2);
    }

    /* Creo le N pipe per i processi figli */
    for (i = 0; i < N; i++)
    {
        if ((pipe(piped[i])) < 0)          /* Controllo che la creazione della pipe vada a buon fine */      
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            
            /* Chiudo tutte le pipe in lettura e anche quella in scrittura se i != j */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if(i != j)
                {
                    close(piped[j][1]);
                }
            }

            if ((pipe(p)) < 0)          /* Controllo che la creazione della pipe per il processo nipote vada a buon fine */
            {
                printf("Errore nella creazione della pipe fra figlio e nipote\n");
                exit(-1);
            }
            
            if ((pid = fork()) < 0)     /* Controllo che la creazione del processo nipote vada a buon fine */
            {
                printf("Errore nella creazione del processo nipote\n");
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */

                /* Chiudo la pipe rimasta aperta tra figlio e padre in lato di scrittura poiche' non serve */
                close(piped[i][1]);
                
                /* Chiudo lo standard output e faccio la dup su p[1]*/
                close(1);
                dup(p[1]);
                
                /* Chiudo la pipe di comunicazione tra processo figlio e nipote */
                close(p[0]);
                close(p[1]);

                /* Faccio la exec sull'iesimo parametro passato */
                execlp("sort", "sort", "-f", argv[i + 1], (char *) 0);

                /* Non si dovrebbe mai arriavare a questo punto
                   In tal caso stampo su standard error che c'e' stato un errore ed esco con -1 */
                perror("Errore nella exec\n");
                exit(-1);

            }

            /* Processo figlio */
            
            /* Chiudo in lato scrittura la pipe nipote in scrittura */
            close(p[1]);

            /* Inizializzo la struct con il pid del nipote*/
            S.pid_nipote = pid;

            j = 0;
            while (read(p[0], &S.linea[j], 1))
            {
                if(S.linea[j] == '\n')
                {
                    S.lunghezza_linea = j + 1;
                    break;
                } 
                else
                {
                    j++;
                }
            }

            /* Il filgio comunica al padre la struct */
            write(piped[i][1], &(S), sizeof(S));
            
            ritorno = -1;       /* Setto di default -1 il valore del ritorno */
            if ((pid = wait(&status)) < 0)      /* Controllo che la wait vada a buon fine */
            {
                printf("Errore nella wait\n");
            }
            else if ((status & 0xFF) != 0)
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
    /* Chiudo nel lato di scrittura tutte le pipe dell'array piped */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }

    /* Recupero le informazioni dai figli */
    for (i = 0; i < N; i++)
    {
        nr = read(piped[i][0], &(S), sizeof(S));
        if (nr != 0)
        {
            S.linea[S.lunghezza_linea] = '\0';
            printf("Il nipote con PID: %d ha letto dal file %s questa linea '%s' che ha lunghezza (compreso il terminatore) %d\n", S.pid_nipote, argv[i + 1], S.linea, S.lunghezza_linea);
        }
    }

    /* Faccio la wait per i processi figli */
    for (i = 0; i < N; i++)
    {
        if ((pid = wait(&status)) < 0)      /* Controllo che la wait abbia successo */
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        else if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio con PID: %d ha ritornato %d (se 255 problemi nel filgio o nipote!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}
