#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di file passati come parametro */
    int C;                          /* Ultimo parametro passato */
    int pid;                        /* Per fork */
    int fd;                         /* Per open */
    int fcreato;                    /* Per creat */
    int nro_totale;                 /* Numero di blocchi totali del file */
    int nro;                        /* Numero di blocchi letti */
    pipe_t *pipes_ps;               /* Array di pipe di comunicazione tra primo e secondo figlio */
    int i, j;                       /* Indici per i cicli */
    char *b;                        /* Array di char che contiene un blocco di dimensione C alla volta */
    char *FCreato;                  /* Nome del file da creare */
    int nr, nw;                     /* Variabili di controllo per read e write */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel passaggio dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Inizializzo N con il numero di file passati come parametro */
    N = argc - 2;

    /* Controllo che l'ultimo carattere sia un intero strettamente positivo */
    if ((C = atoi(argv[argc - 1])) <= 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un intero strettamente positivo\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Alloco spazio per N pipe */
    pipes_ps = (pipe_t *)malloc(N * sizeof(pipe_t));
    if (pipes_ps == NULL)
    {
        printf("Errore nella malloc per l'array di pipe\n");
        exit(3);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipes_ps[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(4);
        }
        
    }
    
    /* Alloco memoria per l'array che contiene i blocchi di caratteri letti da file */
    b = (char *)malloc(C);
    if (b == NULL)
    {
        printf("Errore nella malloc per l'array di char b\n");
        exit(5);
    }
    
    /* Creo i 2 * N figli */
    for (i = 0; i < 2 * N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork per il processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            if (i < N)
            {
                /* Primo processo figlio */
                /* Chiudo le pipe non necessarie */
                for (j = 0; j < N; j++)
                {
                    close(pipes_ps[j][0]);
                    if (j != i)
                    {
                        close(pipes_ps[j][1]);    
                    }
                    
                }
                
                /* Apro in lettura il file associato */
                if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nella open del file %s\n", argv[i + 1]);
                    exit(0);
                }
                
                /* Calcolo da quanti blocchi è composto il file */
                nro_totale = lseek(fd, 0L, 2) / C;
                /* Assegno a nro la metà del numero dei blocchi */
                nro = nro_totale / 2;

                /* Riporto l'I/O pointer all'inizio del file */
                lseek(fd, 0L, 0);

                /* Itero un ciclo che legge un blocco di caratteri alla volta e che termina una volta che sono stati letti nro blocchi */
                for (j = 0; j < nro; j++)
                {
                    read(fd, b, C);
                    /* Invio il blocco al secondo processo figlio */
                    nw = write(pipes_ps[i][1], b, C);
                    if (nw != C)
                    {
                        printf("Errore: processo figlio di indice i = %d ha scritto un numero sbagliato di bytes su pipe %d\n", i, nw);
                        exit(0);
                    }
                    
                }
                
            }
            else
            {
                /* Secondo processo figlio */
                /* Creo il file */
                FCreato = (char *)malloc((strlen(argv[i - N + 1]) + 11));
                if (FCreato == NULL)
                {
                    printf("Errore nella malloc per l'array di char FCreato\n");
                    exit(0);
                }
                strcpy(FCreato, argv[i - N + 1]);
                strcat(FCreato, ".mescolato");

                if ((fcreato = creat(FCreato, 0644)) < 0)
                {
                    printf("Errore nella creazione del file %s\n", FCreato);
                    exit(0);
                }
                
                /* Chiudo le pipe non necessarie */
                for (j = 0; j < N; j++)
                {
                    close(pipes_ps[j][1]);
                    if (j != i - N)
                    {
                        close(pipes_ps[j][0]);
                    }
                    
                }
                
                /* Apro in lettura il file */
                if ((fd = open(argv[i - N + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file %s\n", argv[i - N + 1]);
                    exit(0);
                }
                
                /* Calcolo da quanti blocchi è composto il file */
                nro_totale = lseek(fd, 0L, 2) / C;
                /* Assegno a nro la metà del numero dei blocchi */
                nro = nro_totale / 2;

                /* Riporto l'I/O pointer alla metà del file */
                lseek(fd, (long int)C * nro, 0);

                /* Leggo i blocchi della seconda metà*/
                for (j = 0; j < nro; j++)
                {
                    read(fd, b, C);

                    /* Scrivo il blocco letto sul file creato */
                    write(fcreato, b, C);

                    /* Leggo da pipe il blocco inviato dal primo processo figlio */
                    nr = read(pipes_ps[i - N][0], b, C);
                    if (nr != C)
                    {
                        printf("Errore in lettura da pipe %d\n", i-N);
						exit(0);
                    }
                    
                    /* Scrivo il blocco letto sul file creato */
                    write(fcreato, b, C);
                }
                
            }

            /* Ritorno il numero di blocchi letti da ogni processo figlio */
            exit(nro);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipes_ps[i][0]);
        close(pipes_ps[i][1]);
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N * 2; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d ritornato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 0 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}