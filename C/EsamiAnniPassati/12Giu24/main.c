#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{

    /* ------ Varaibili locali ------ */
    int N;                          /* Numero di parametri */
    int pid;                        /* Per fork e wait */
    int fd;                         /* File descriptor per la open in lettura del file */
    int fcreato;                    /* File descriptor per il file da creare */
    char *Fcreato;                  /* Stringa del file da creare */
    pipe_t *pipe_ps;                /* Array di pipe di comunicazione tra primo e secondo processo figlio */
    int n, i;                       /* Indici per i cicli */
    int Nlinea;                     /* Variabile che conta a che numero di linea ci si trova */
    int nro;                        /* Variabile che contiene il massimo numero di caratteri letti in una linea */
    char linea[250];                /* Buffer di lettura della linea */
    int iLetto;                     /* Lunghezza letta da pipe */
    char lineaLetta[250];           /* Linea letta da pipe */
    int status, ritorno;            /* Per wait */
    /* ------------------------------ */

    /* Controllo che sia passato almeno 1 parametro */
    if (argc < 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 1 parametro (nomefile1, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo nella variabile N il numero di parametri passati */
    N = argc - 1;

    /* Alloco memoria per l'array di N pipe */
    pipe_ps = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (pipe_ps == 0)
    {
        printf("Errore nella malloc\n");
        exit(2);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione dell'i-esima pipe vada a buon fine */
        if (pipe(pipe_ps[i]) < 0)
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(3);
        }
    }
    
    /* Creo i 2*N processi figli */
    for (n = 0; n < 2*N; n++)
    {
        /* Controllo che la creazione dell'n-esimo processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Controllo se sono nel primo o nel secondo processo associato al file */
            if (n < N)
            {
                /* Primo processo figlio */
                /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
                /* Chiudo i lati della pipe che non servono */
                for (i = 0; i < N; i++)
                {
                    close(pipe_ps[i][0]);
                    if (i != n)
                    {
                        close(pipe_ps[i][1]);
                    }
                }
                
                /* Apro in lettura il file associato */
                if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura in lettura del file %s\n", argv[n + 1]);
                    exit(-1);
                }

                /* Setto le variabili i, Nlinea e nro */
                i = 0;              /* Indice di linea */
                Nlinea = 0;         /* Numero di linea corrente */
                nro=-1;             /* Numero massimo di caratteri nella linea */
                /* Itero un ciclo che legge tutti i caratteri del file */
                while (read(fd,&(linea[i]), 1))
                {
                    /* Controllo se sono arrivato alla fine di una linea */
                    if (linea[i] == '\n')
                    {
                        /* Incremento il numero di linea e controllo se sono in una linea dispari */
                        Nlinea++;
                        if ((Nlinea % 2) == 1)
                        {
                            /* Aggiungo il terminatore alla lunghezza della linea e invio le informazioni al secondo processo figlio */
                            i++;
                            write(pipe_ps[n][1], &i, sizeof(i));
                            write(pipe_ps[n][1], linea, i);
                            
                            /* Controllo se i > nro */
                            if (i > nro)
                            {
                                nro = i;
                            }
                        }
                        
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                }   
            }
            else
            {
                /* Secondo processo figlio */
                /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
                /* Alloco memoria per la stringa del file da creare */
                Fcreato = (char *)malloc((strlen(argv[n - N + 1] + 5)));
                /* Controllo che la malloc sia andata a buon fine */
                if (Fcreato == NULL)
                {
                    printf("Errore nella malloc della stringa\n");
                    exit(-1);
                }
                
                /* Creo la stringa Fcreato */
                strcpy(Fcreato, argv[n - N + 1]);
                strcat(Fcreato, ".max");

                /* Creo il file */
                if ((fcreato = creat(Fcreato, PERM)) < 0)
                {
                    printf("Errore nella creazione del file %s\n", Fcreato);
                    exit(-1);
                }
                
                /* Chiudo i lati della pipe che non servono */
                for (i = 0; i < N; i++)
                {
                    close(pipe_ps[i][1]);
                    if (i != n - N)
                    {
                        close(pipe_ps[i][0]);
                    }
                }
                
                /* Apro il file associato in lettura */
                if ((fd = open(argv[n - N + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura in lettura del file %s\n", argv[n - N + 1]);
                    exit(-1);
                }
                
                /* Setto le variabili i, Nlinea e nro */
                i = 0;
                Nlinea = 0;
                nro = -1;
                /* Itero un ciclo che legge tutti i  caratteri del file */
                while (read(fd, &(linea[i]), 1))
                {
                    /* Controllo se sono arrivato alla fine di una linea */
                    if (linea[i] == '\n')
                    {
                        Nlinea++;

                        /* Controllo se sono in una linea pari */
                        if ((Nlinea % 2) == 0)
                        {
                            /* Aggiungo il terminatore alla lunghezza */
                            i++;
                            /* Recupero le informazioni dal primo processo figlio */
                            read(pipe_ps[n - N][0], &iLetto, sizeof(iLetto));
                            read(pipe_ps[n - N][0], lineaLetta, iLetto);
                            if (iLetto > i)
                            {
                                write(fcreato, lineaLetta, iLetto);
                            }
                            else
                            {
                                write(fcreato, linea, i);
                            }
                            
                            /* Controllo se i > nro */
                            if (i > nro)
                            {
                                nro = i;
                            }
                        }
                        
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                }
            }

            exit(nro);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipe_ps[i][0]);
        close(pipe_ps[i][1]);
    }

    /* Il padre aspetta i figli */
    for (n = 0; n < 2*N; n++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e'terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}