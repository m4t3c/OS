#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

/* Funzione che calcola un numero casuale compreso tra 0 e n - 1 */
int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int H;                              /* Variabile che rappresenta il numero di righe dei file */
    int N;                              /* Numero di file passati come parametro */
    int pid;                            /* Per fork */
    pipe_t *pipe_pf;                    /* Pipe di comunicazione tra padre e figlio */
    pipe_t *pipe_fp;                    /* Pipe di comunicazione tra figlio e padre */
    int fdout;                          /* File descriptor per il file che creiamo */
    int fd;                             /* File descriptor per la open del file associato al processo */
    int i, j;                           /* Indici per i cicli */
    char linea[255];                    /* Buffer che contiene ogni linea letta */
    int r;                              /* Valore inviato dal padre */
    int ritorno = 0;                    /* Ogni processo figlio ritorna il numero di caratteri scritti sul file creato */
    int valore;                         /* Variabile che salva il valore inviato dal filgio al padre */
    int status;                         /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 5 parametri */
    if (argc < 6)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 5 parametri (file1, file2, file3, file4, ..., H) ma argc = %d\b", argc);
        exit(1);
    }
    
    /* Salvo in H l'ultimo parametro passato */
    H = atoi(argv[argc - 1]);
    /* Controllo che l'ultimo parametro sia un numero intero strettamente positivo minore di 255 */
    if ((H < 0) || (H >= 255))
    {
        printf("Errore nel passaggio dei parametri: %s non e' un intero strettamente positivo e minore di 255\n", argv[argc - 1]);
        exit(2);
    }

    /* Salvo il numero dei file nella variabie N */
    N = argc - 2;

    /* Inizializzo il seme per la generazione random di numeri */
    srand(time(NULL));

    /* Creo il file /tmp/creato*/
    if ((fdout = open("/tmp/creato", O_CREAT| O_WRONLY | O_TRUNC, PERM)) < 0)
    {
        printf("Errore nella creazione del file %s\n", "/tmp/creato");
        exit(3);
    }
    
    /* Alloco la memoria per gli array delle due pipe di comunicazione */
    pipe_pf = (pipe_t*)malloc(N * sizeof(pipe_t));
    pipe_fp = (pipe_t*)malloc(N * sizeof(pipe_t));

    /* Controllo che l'allocazione di memoria per gli array di pipe sia andata a buon fine */
    if ((pipe_pf == NULL) || (pipe_fp == NULL))
    {
        printf("Errore nella malloc per gli array di pipe\n");
        exit(4);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipe_pf[i]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra il processo padre e il processo figlio di indice i = %d\n", i);
            exit(5);
        }
        
        if (pipe(pipe_fp[i]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra il processo figlio e il processo padre di indice i = %d\n", i);
            exit(6);
        }        
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(7);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare in caso di errore -1 che corrisponde a 255 senza segno */
            /* Chiudo tutte le pipe non necessarie per l'i-esimo processo figlio */
            for (j = 0; j < N; j++)
            {
                close(pipe_fp[j][0]);
                close(pipe_pf[j][1]);
                if (i != j)
                {
                    close(pipe_fp[j][1]);
                    close(pipe_pf[j][0]);
                }
            }
            
            /* Apro in lettura il file associato al processo */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Setto a 0 j che fungera' da contatore nel ciclo di lettura da file */
            j = 0;

            /* Itero un ciclo che legge carattere per carattere da file */
            while (read(fd, &linea[j], 1))
            {
                /* Controllo se sono arrivato alla fine di una linea */
                if (linea[j] == '\n')
                {
                    /* Mando al padre la lunghezza della linea corrente compreso il terminatore */
                    j++;
                    write(pipe_fp[i][1], &j, sizeof(j));

                    /* Leggo il valore inviato dal padre */
                    read(pipe_pf[i][0], &r, sizeof(r));
                    if (r < j)
                    {
                       write(fdout, &(linea[r]), 1);
                       ritorno++; 
                    }
                    j = 0;
                }
                else
                {
                    j++;
                }
            }

            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe che non mi servono */
    for (i = 0; i < N; i++)
    {
        close(pipe_pf[i][0]);
        close(pipe_fp[i][1]);   
    }
    
    /* Itero un ciclo che mi permette di recuperare le informazioni dai figli per poi inviarle */
    for (j = 1; j < H; j++)
    {
        r = mia_random(N);
        for (i = 0; i < N; i++)
        {
            if (i == r)
            {
                read(pipe_fp[i][0], &valore, sizeof(valore));
            }
        }
        
        /* Calcolo un indice random della riga che mi sono salvato */
        r = mia_random(valore);
        /* Invio a tutti i figli l'indice da considerare */
        for (i = 0; i < N; i++)
        {
            write(pipe_pf[i][1], &r, sizeof(r));
        }
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
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
