#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Definisco la funzione mia_random */
#include <stdlib.h>
int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di file passati come parametro */
    int H;                          /* Lunghezza in linee dei file */
    int pid;                        /* Per fork */
    int fd;                         /* Per open */
    int Fcreato;                    /* File descriptor file creato */
    int n, i;                       /* Indici */
    pipe_t *pipes_pf;               /* Array di pipe tra padre e figlio */
    pipe_t *pipes_fp;               /* Array di pipe tra figlio e padre */
    char linea[255];                /* Buffer per la lettura della linea */
    int valore;                     /* Valore recuperato dal padre che contiene la lunghezza della linea corrente */
    int giusto;                     /* Valore dell'indice del processo figlio di cui si userà la lunghezza della linea per generare l'indice random */
    int r;                          /* Indice generato random */
    int ritorno = 0;                /* Numero di caratteri scritti da ogni processo figlio sul file creato, inizialmente 0 */
    int pidFiglio, status;          /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 5 parametri */
    if (argc < 6)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 5 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di file passati come parametri */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un intero strettamente positivo e minore di 255 */
    H = atoi(argv[argc - 1]);

    if (H <= 0 || H >= 255)
    {
        printf("Errore: %s non è un numero intero strettamente positivo oppure non è strettamente minore di 255\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Inizializzo il seme per la generazione casuale di numeri */
    srand(time(NULL));

    /* Creo/apro il file su /tmp */
    if ((Fcreato = open("/tmp/creato", O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
        printf("Errore nella open del file /tmp/creato\n");
        exit(3);
    }
    
    /* Alloco memoria per i due array di pipe */
    pipes_fp = (pipe_t *) malloc(N * sizeof(pipe_t));
    pipes_pf = (pipe_t *) malloc(N * sizeof(pipe_t));
    if ((pipes_fp == NULL) || (pipes_pf == NULL))
    {
        printf("Errore nella malloc per gli array di pipe\n");
        exit(4);
    }
    
    /* Creo le N pipe */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione dell'n-esima pipe tra padre e figlio vada a buon fine */
        if (pipe(pipes_pf[n]) < 0)
        {
            printf("Errore nella creazione della pipe tra padre e figlio di indice n = %d\n", n);
            exit(5);
        }
        
        /* Controllo che la creazione dell'n-esima pipe tra figlio e padre vada a buon fine */
        if (pipe(pipes_fp[n]) < 0)
        {
            printf("Errore nella creazione della pipe tra figlio e padre di indice n = %d\n", n);
            exit(6);
        }
        
    }
    
    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(7);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe che non servono */
            for (i = 0; i < N; i++)
            {
                close(pipes_fp[i][0]);
                close(pipes_pf[i][1]);
                if (i != n)
                {
                    close(pipes_fp[i][1]);
                    close(pipes_pf[i][0]);
                }
                
            }
            
            /* Apro il file associato al processo figlio in lettura */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nella open di %s\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Inizializzo i a 0 */
            i = 0;

            /* Leggo il file un carattere alla volta riga per riga */
            while (read(fd, &linea[i], 1))
            {
                /* Controllo se sono arrivato alla fine della riga */
                if (linea[i] == '\n')
                {
                    /* Comunico al padre la lunghezza della riga compreso il terminatore */
                    i++;
                    write(pipes_fp[n][1], &i, sizeof(i));

                    /* Leggo il numero inviato dal padre e lo salvo in r */
                    read(pipes_pf[n][0], &r, sizeof(r));

                    /* Controllo se l'indice inviato dal padre è contenuto nella linea corrente */
                    if (r < i)
                    {
                        /* Scrivo il carattere sul file creato */
                        write(Fcreato, &linea[r], 1);
                        
                        /* Incremente il numero di ritorno che indicherà il numero di caratteri scritti sul file da ogni filgio */
                        ritorno++;
                    }
                    
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
            
            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe non necessarie */
    for (n = 0; n < N; n++)
    {
        close(pipes_pf[n][0]);
        close(pipes_fp[n][1]);
    }
    
    /* Il padre recupera le informazioni dai figli e invia l'indice per tutte le H righe */
    for (i = 0; i < H; i++)
    {
        /* Genero l'indice random di cui bisogna salvare la lunghezza della riga corrente */
        r = mia_random(N);

        /* Recupero le informazioni da tutti i figli e salvo in giusto solo quello che ha scritto il filgio numero r */
        for (n = 0; n < N; n++)
        {
            read(pipes_fp[n][0], &valore, sizeof(valore));
            
            /* Controllo se n = r */
            if (n == r)
            {
                giusto = valore;
            }
            
        }
        
        /* Calcolo l'indice random */
        r = mia_random(giusto);

        /* Invio a tutti i figli l'indice random */
        for (n = 0; n < N; n++)
        {
            write(pipes_pf[n][1], &r, sizeof(r));
        }
        
    }

    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d ritornato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}