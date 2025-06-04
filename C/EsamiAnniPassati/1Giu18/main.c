#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di due interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------- */
    int N;                          /* Numero di file/processi figli */
    int K;                          /* Ultimo parametro */
    int X;                          /* Valore digitato dall'utente */
    int *pid;                       /* Array di pid per la fork */
    pipe_t *piped;                  /* Array di pipe */
    int i, j;                       /* Indici per i cicli */
    int fd;                         /* File descriptor per il file del processo figlio */
    int nlinea = 0;                 /* Contatore del numero di linea corrente */
    int trovato = 0;                /* Variabile che indica se si e' trovata la X-esima linea */
    char linea[255];                /* Buffer di lettura della linea corrente */
    int ritorno;                    /* Per processo figlio e wait */
    int nr;                         /* Per read */
    int pidFiglio, status;          /* Per wait */

    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, ..., K) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Assegno ad N il numero di file passati come parametro */
    N = argc - 2;

    /* Controllo se l'ultimo parametro e' un numero intero strettamente positivo */
    if ((K = atoi(argv[argc - 1])) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un numero intero strettamente positivo\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Alloco memoria per l'array di pipe */
    piped = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (piped == NULL)
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(3);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la i-esima pipe sia creata correttamente */
        if (pipe(piped[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(4);
        }
    }
    
    /* Alloco memoria per l'array di pid */
    pid = (int *)malloc(N * sizeof(int));
    /* Controllo che la malloc sia andata a buon fine */
    if (pid == NULL)
    {
        printf("Errore nella malloc dell'array di pid\n");
        exit(5);
    }
    
    /* Leggo dall'utente un valore positivo minore o uguale a K */
    printf("Digita un numero maggiore di 0 e minore o uguale a %d:\n", K);
    scanf("%d", &X);
    /* Controllo che il valore sia valido */
    if ((X <= 0) || (X > K))
    {
        printf("Valore digitato non valido\n");
        exit(6);
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(7);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo tutte le pipe che non servono */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if (j != i)
                {
                    close(piped[j][1]);
                }
            }
            
            /* Apro in lettura il file associato al processo figlio */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Inizializzo j a 0 che fungera' da indice */
            j = 0;
            /* Leggo tutti i caratteri del file */
            while (read(fd, &(linea[j]), 1))
            {
                if (linea[j] == '\n')
                {
                    if (nlinea == X)
                    {
                        trovato = 1;
                        j++;
                        ritorno = j;
                        break;
                    }
                    else
                    {
                        nlinea++;
                        j = 0;
                    }                  
                }
                else
                {
                    j++;
                }
            }
            
            /* Controllo se e' stata trovata la X-esima riga */
            if (!trovato)
            {
                sprintf(linea, "Il file %s non contiene la linea %d-esima\n", argv[i + 1], X);
                j = strlen(linea);
                ritorno = 0;
            }
            write(piped[i][1], &j, sizeof(j));
            write(piped[i][1], linea, j);

            exit(ritorno);
        }               
    }
    	
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }
    
    /* Il padre recupera le informazioni dai figli */
    for (i = 0; i < N; i++)
    {
        nr = read(piped[i][0], &j, sizeof(j));
        if (nr == 0)
        {
            printf("Il processo figlio di indice i = %d e PID: %d ha avuto problemi\n", i, pid[i]);
        }
        else
        {
            read(piped[i][0], linea, j);
            linea[j - 1] = '\0';
            printf("Il processo figlio di indice i = %d e PID: %d ha comunicato questo %s\n", i, pid[i], linea);
        }
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d e' terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}