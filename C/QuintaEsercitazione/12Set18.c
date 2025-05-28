#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

int main(int argc, char ** argv)
{
    /* ----- Variabili locali ----- */
    int N;                          /* Numero di parametri passati */
    int pid;                        /* Per fork */
    int i, j;                       /* Indici dei cicli */
    pipe_t* pipe_pf;                /* Pipe di comunicazione tra processo padre e processo figlio */
    pipe_t* pipe_pn;    	        /* Pipe di comunicazione tra processo padre e processo nipote */
    int fd;                         /* Variabile per la open */
    char c;                         /* Buffer del carattere letto */
    long int trasformazioni = 0L;   /* Variabile che conta il numero di trasformazioni effettuate nel processo figlio e nel processo nipote */
    int ritorno;                    /* Variabile di ritorno dei processi figlio e nipote */
    int nr;                         /* Variabile di controllo dei caratteri letti */
    int status;                     /* Per wait */
    /* ---------------------------- */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomi di file) ma argc = %d\n", argc);
        exit(1);
    }

    N = argc - 1;
    
    /* Alloco memoria per i due array di pipe */
    pipe_pf = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipe_pn = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che l'allocazione di memoria sia andata a buon fine */
    if ((pipe_pf == NULL) || (pipe_pn == NULL))
    {
        printf("Errore nella malloc degli array di pipe\n");
        exit(2);
    }

    /* Creo le N pipe sia padre figlio che padre nipote */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esima pipe abbia successo */
        if ((pipe(pipe_pf[i]) < 0) || (pipe(pipe_pn[i]) < 0))
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }

    /* Itero un ciclo che crea N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esimo processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazone del processo figlio di indice i = %d\n", i);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* In caso di errore verrà usato il valore -1 (255 senza segno) per comunicarlo al processo padre */
            /* Chiudo tutte le pipe che non mi servono*/
            for (j = 0; j < N; j++)
            {
                close(pipe_pf[j][0]);
                if (i != j)
                {
                    close(pipe_pf[j][1]);
                }
            }
            
            /* Creo l'i-esimo processo nipote */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella creazione del processo nipote di indice i = %d\n", i);
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* In caso di errore verrà usato il valore -1 (255 senza segno) per comunicarlo al processo padre */
                /* Chiudo tutte le pipe che non mi servono*/
                for (j = 0; j < N; j++)
                {
                    close(pipe_pn[j][0]);
                    if (i != j)
                    {
                        close(pipe_pn[j][1]);
                    }
                }

                /* Apro il file il modalità scrittura lettura */
                /* Controllo che l'apertura vada a buon fine */
                if ((fd = open(argv[i + 1], O_RDWR)) < 0)
                {
                    printf("Errore nell'apertura del file %s nel processo nipote\n", argv[i + 1]);
                    exit(-1);
                }
                
                /* Leggo un carattere alla volta dal file e cerco i caratteri alfabetici minuscoli */
                while (read(fd, &c, 1))
                {
                    /* Nel caso fosse un carattere alfabetico minuscolo devo incrementare trasformazioni e devo trasformarlo in un carattere alfabetico maiuscolo */
                    if (islower(c))
                    {
                        trasformazioni++;
                        c = toupper(c);
                        lseek(fd, -1L, SEEK_CUR);
                        write(fd, &c, 1);                       
                    }
                }

                write(pipe_pn[i][1], &trasformazioni, sizeof(trasformazioni));
                /* Setto la variabile ritorno a:
                        0: se sono state fatte meno di 256 trasformazioni
                        1: se sono state fatte 256 trasformazioni ma meno di 512 
                        2: se sono state fatte 512 trasformazioni ma meno di 768
                        ...
                */
                ritorno = trasformazioni/256;
                exit(ritorno);
            }

            /* Processo figlio */
            /* Chiudo tutte le pipe del nipote */
            for (j = 0; j < N; j++)
            {
                close(pipe_pn[j][0]);
                close(pipe_pn[j][1]);
            }

            /* Apro in lettura e scrittura il file associato */
            if ((fd = open(argv[i + 1], O_RDWR)) < 0)
            {
                printf("Errore nell'apertura del file %s nel processo figlio di indice i = %d\n", argv[i + 1], i);
                exit(-1);
            }
            
            /* Itero un ciclo che legge un carattere alla volta */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è un numero */
                if(isdigit(c))
                {
                    /* Cambio il carattere in uno spazio e aumento il numero di trasformazioni */
                    c = ' ';
                    lseek(fd, -1L, SEEK_CUR);
                    write(fd, &c, 1);
                    trasformazioni++;
                }
            }

            /* Invio al padre il numero di trasformazioni effettuate */
            write(pipe_pf[i][1], &trasformazioni, sizeof(trasformazioni));

            /* Il processo figlio aspetta la terminazione del nipote */
            if((pid = wait(&status)) < 0)
            {
                printf("Errore nella wait\n");
            }
            if((status & 0xFF) != 0)
            {
                printf("Il processo nipote con PID: %d e' terminato in modo anomalo\n", pid);
            }
            else
            {
                ritorno = (int) ((status >> 8) & 0xFF);
                printf("il nipote con PID: %d ha ritornato %d\n", pid, ritorno);
            }

            ritorno = trasformazioni/256;
            exit(ritorno);
        }
    }

    /* Codice del padre */
    /* Chiudo tutte le pipe che non servono */
    for (i = 0; i < N; i++)
    {
        close(pipe_pf[i][1]);
        close(pipe_pn[i][1]);
    }
    
    /* Recupero le informazioni dalle pipe dei processi figli e nipoti */
    for (i = 0; i < N; i++)
    {
        nr = read(pipe_pf[i][0], &trasformazioni, sizeof(trasformazioni));
        if (nr == sizeof(trasformazioni))
        {
            printf("Il processo figlio di indice i = %d ha effettuato %ld trasformazioni sul file %s\n", i, trasformazioni, argv[i + 1]);
        }
        else
        {
            printf("Errore nella lettura del figlio di indice i = %d\n", i);
        }

        nr = read(pipe_pn[i][0], &trasformazioni, sizeof(trasformazioni));
        if (nr == sizeof(trasformazioni))
        {
            printf("Il processo nipote di indice i = %d ha effettuato %ld trasformazioni sul file %s\n", i, trasformazioni, argv[i + 1]);
        }
        else
        {
            printf("Errore nella lettura del nipote di indice i = %d\n", i);
        }
    }
       
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        if((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
        }
        if((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int) ((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d e' terminato con valore %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}