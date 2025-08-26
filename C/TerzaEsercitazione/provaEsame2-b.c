#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di caratteri */
    int n;                          /* Indice dei processi figli */
    int i;                          /* Indice */
    int fd;                         /* Per la open */
    int pid;                        /* Per fork */
    pipe_t *piped;                  /* Array di N pipe */
    long int occ;                   /* Numero di occorrenze del carattere nel file */
    char c;                         /* Carattere associato ad ogni figlio */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di caratteri passati come parametro in N */
    N = argc - 2;

    /* Controllo che tutti gli N caratteri siano singoli caratteri */
    for (n = 0; n < N; n++)
    {
        if (strlen(argv[n + 2]) != 1)
        {
            printf("Errore nel passaggio dei parametri: %s non è un singolo carattere\n", argv[n + 2]);
            exit(2);
        }
        
    }
    
    /* Alloco l'array di N pipe */
    if ((piped = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(3);
    }
    

    /* Creo N pipe di comunicazione tra padre e figlio */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(piped[n]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", n);
            exit(4);
        }
        
    }

    /* Creo N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork del processo figlio di indice n = %d\n", n);
            exit(5);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe inutilizzate */
            for (i = 0; i < N; i++)
            {
                close(piped[i][0]);
                if (i != n)
                {
                    close(piped[i][1]);
                }
                
            }
            
            /* Apro in lettura il file passato come primo parametro */
            if ((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nella open del file %s\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Inizializzo occ a 0 */
            occ = 0L;

            /* Itero un ciclo che legge tutti i caratteri del file */
            while (read(fd, &c, 1) > 0)
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == argv[n + 2][0])
                {
                    /* Incremento il numero di occorrenze del carattere */
                    occ++;
                }
                
            }
            
            /* Scrivo sulla pipe il numero di occorrenze */
            write(piped[n][1], &occ, sizeof(occ));

            exit(argv[n + 2][0]);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe non utilizzate */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }
    
    /* Leggo dalla pipe i messaggi scritti dai figli */
    for (n = 0; n < N; n++)
    {
        if (read(piped[n][0], &occ, sizeof(occ)) > 0)
        {
            printf("Nel file %s sono state trovate %ld occorrenze del carattere %c\n", argv[1], occ, argv[n + 2][0]);
        }
        
    }
    
    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(6);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo filgio con PID: %d è terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %c (in decimale %d se 255 problemi!)\n", pidFiglio, ritorno, ritorno);
        }
    }
 
    exit(0);
}