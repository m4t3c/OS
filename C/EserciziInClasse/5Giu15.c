#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];              /* Definisco il tipo pipe_t come array di 2 interi */

int main(int argc, char **argv)
{
    /*----- Variabili locali -----*/
    int pid;                        /* Per fork */
    int N;                          /* Numero di parametri passati */
    int i, j;                       /* Indice dei cicli */
    int lenght;                     /* Variabile per il processo figlio */  
    int l;  	                    /* Variabile per contare il numero di caratteri letti */
    pipe_t *piped;                  /* Array di pipe */
    pipe_t p;                       /* Pipe singola per comunicazione processo figlio e processo nipote */
    char ch;                        /* Variabile in cui salvare il carattere letto */
    int pidFiglio, ritorno, status; /* Per wait */
    /*----------------------------*/
    
    if(argc < 3)                    /* Controllo che il numero di parametri sia corretto */
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomi assoluti di file) ma argc = %d\n", argc);
        exit(1);
    }

    N = argc - 1;

    /* Alloco memoria per N pipe */
    piped = (pipe_t *) malloc(N * sizeof(pipe_t));
    if(piped == NULL)               /* Controllo che la malloc sia andata a buon fine */
    {
        printf("Errore nella malloc\n");
        exit(2);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if((pipe(piped[i])) < 0)    /* Controllo che la creazione della i-esima pipe sia andata a buon fine */
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        if((pid = fork()) < 0)      /* Controllo che la fork sia andata a buon fine */
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(4);
        }
        if(pid == 0)
        {
            /* Codice del figlio */
            /* Chiudo tutte le pipe in lato di lettura e quelle che non mi servono in lato di scrittura */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if(i != j)
                {
                    close(piped[j][1]);
                }
            }
            
            /* Creo la pipe per il processo nipote */
            if(pipe(p) < 0)         /* Controllo che la pipe sia creata con successo */
            {
                printf("Errore nella creazione della pipe di comunicazione tra processo figlio e processo nipote\n");
                exit(-1);
            }

            /* Creo il processo nipote */
            if((pid = fork()) < 0)  /* Controllo che la fork sia andata a buon fine */
            {
                printf("Errore nella fork di creazione del processo nipote di indice i = %d\n", i);
                exit(-2);
            }
            if(pid == 0)
            {
                /* Processo nipote */
                /* Chiudo la pipe in scrittura tra padre e figlio */
                close(piped[i][1]);
                
                /* Chiudo standard output e duplico la pipe in scrittura */
                close(1);
                dup(p[1]);

                /* Chiudo su entrambi i lati la pipe figlio-nipote */
                close(p[0]);
                close(p[1]);

                /* Eseguo la tail sul parametro passsato come i-esimo */
                execlp("tail", "tail", "-1", argv[i + 1], (char *)0);
                
                /* Non dovremmo mai arrivare quì */
                perror("Errore nella esecuzione della tail nel processo nipote");
                exit(-3);
            }

            /* Processo figlio */
            close(p[1]);

            /* Inizializzo l a 0 */
            l = 0;

            /* Leggo tutti i caratteri da p[0] */
            while (read(p[0], &ch, 1))
            {
                ++l;
            }
            
            if(l != 0)      /* Nel caso in cui sono stati letti dei caratteri salvo la loro lunghezza meno il terminatore in lunghezza */
            {
                lenght = l;
            }
            else
            {
                lenght = 0;
            }

            /* Comunico al padre il numero di caratteri letti */
            write(piped[i][1], &lenght, sizeof(lenght));

            ritorno = -1;    /* Di defalt lascio la variabile ritorno come se fosse avvenuto un errore */
            
            if((pid = wait(&status)) < 0)   /* Controllo che la wait abbia successo */
            {
                printf("Errore nella wait del processo nipote di indice i = %d\n", i);
            }
            else if ((status & 0xFF) != 0)
            {
                printf("Processo nipote di PID: %d terminato in modo anomalo\n", pid);
            }
            else
            {
                ritorno = (int)((status >> 8) & 0xFF);
            }
            
            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe in scrittura */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }

    /* Recupero le informazioni dalle pipe nel lato di lettura e stampo su standard output */
    for (i = N - 1; i >= 0; i--)
    {
        read(piped[i][0], &lenght, sizeof(lenght));
        printf("Il figlio di indice %d ha comunicato il valore %d\n", i, lenght);
    }

    /* Ciclo che aspetta i processi figli */
    for (i = 0; i < N; i++)
    {
        if((pidFiglio = wait(&status)) < 0) /* Controllo che la wait abbia successo */
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        else if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}