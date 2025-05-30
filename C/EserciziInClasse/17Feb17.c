#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int Q;                              /* Numero di parametri passati */
    int pid;                            /* Per fork */
    pipe_t *pipes;                      /* Array di pipe il fatto che ogni figlio scriva su (q + 1) % Q rende ciclica la pipe ossia l'ultimo scriverà sulla pipe del primo ecc ecc */
    int q, j;                           /* Indici dei cicli */
    int fd; 	                        /* Per la open */
    char linea[250];                    /* Buffer della linea letta dal file */
    char ok;                            /* Messaggio per poter proseguire nella scrittura su standard output del filgio corrente */
    int nnum;                           /* Numero di caratteri numerici nella linea */
    int nr, nw;                         /* Per read e write */
    int ritorno;                        /* Valore di ritorno di ogni porcesso figlio e variabile per wait */
    int pidFiglio, status;              /* Per wait */
    /* ------------------------------ */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, nomefile2, ... ) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri nella variabile Q */
    Q = argc - 1;

    /* Alloco memoria per le Q pipe */
    pipes = (pipe_t *)malloc(Q * sizeof(pipe_t));
    /* Controllo che l'allocazione sia andata a buon fine */
    if (pipes == NULL)
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(2);
    }
    
    /* Creo le Q pipe */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la creazione della q-esima pipe vada a buon fine */
        if (pipe(pipes[q]) < 0)
        {
            printf("Errore nella creazione della pipe di indice q = %d\n", q);
            exit(3);
        }
    }
    
    /* Creo i Q processi figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la creazione del q-esimo processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice q = %d\n", q);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo tutte le pipe di cui non ho bisogno */
            for (j = 0; j < Q; j++)
            {
                if (j != q)
                {
                    close(pipes[j][0]);
                }

                if (j != (q + 1)%Q)
                {
                    close(pipes[j][1]);
                }
            }
            
            /* Apro il file associato al processo figlio corrente */
            if ((fd = open(argv[q + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[q + 1]);
                exit(-1);
            }

            /* Inizializzo l'indice della linea j a 0 e anche il contatore di numeri */
            j = 0;
            nnum = 0;
            
            /* Itero un ciclo che legge tutti i caratteri del file */
            while (read(fd, &(linea[j]), 1) != 0)
            {
                /* Controllo di non essere arrivato alla fine di una linea */
                if (linea[j] == '\n')
                {
                    /* Controllo se ho l'ok dal figlio precedente per stampare su standard output */
                    nr = read(pipes[q][0], &ok, sizeof(char));
                    if (nr != sizeof(char))
                    {
                        printf("Errore: figlio di indice %d ha letto un numero di byte sbagliati %d\n", q, nr);
                        exit(-1);
                    }
                    
                    linea[j] = '\0';
                    printf("Figlio con indice %d e pid %d ha letto %d caratteri numerici nella linea %s\n", q, getpid(), nnum, linea);

                    /* Do l'ok al figlio successivo di stampare */
                    nw = write(pipes[(q +1 ) % Q][1], &ok, sizeof(char));
                    /* Controllo che sia stato scritto correttamente l'ok */
                    if (nw != sizeof(char))
                    {
                        printf("Errore: figlio di indice %d ha letto un numero di byte sbagliati %d\n", q, nw);
                        exit(-1);
                    }
                    
                    j = 0;
                    ritorno = nnum;
                    nnum = 0;
                }
                else
                {
                    if (isdigit(linea[j]))
                    {
                        nnum++;
                    }
                    
                    j++;
                }
            }
            
            exit(ritorno);
        }
    }
    
    /* Processo padre */
    /* Chiudo tutte le pipe che non mi servono */
    for (q = 1; q < Q; q++)
    {
        close(pipes[q][0]);
        close(pipes[q][1]);
    }
    
    /* Mando l'ok al primo figlio */
    nw = write(pipes[0][1], &ok, sizeof(char));
    /* Controllo che la scrittura sia andata a buon fine */
    if (nw != sizeof(char))
    {
        printf("Errore: il padre ha scritto un numero di byte sbagliati %d\n", nw);
        exit(5);
    }
    
    /* Chiudo anche la prima pipe in scrittura */
    close(pipes[0][1]);

    /* Il padre aspetta i figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la wait vada a buon fine */
        if((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(6);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}