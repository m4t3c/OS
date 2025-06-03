#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
    /* ------ Variabili locali ------ */
    int Q;                          /* Numero di parametri/processi figli */
    int filecreato;                 /* File descriptor per il file creato */
    int fd;                         /* File descriptor del file aperto in lettura nei processi figli */
    int pid;                        /* Per fork e wait */
    pipe_t *piped;                  /* Array di pipe */
    int q, i;                       /* Indici per i cicli */
    char linea[250];                /* Buffer per ogni linea letta */
    int ritorno = 0;                /* Valore di ritorno dei processi figli e per wait */
    int status;                     /* Per wait */
    
    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomefile1, nomefile2, ...) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri nella variabile Q */
    Q = argc - 1;

    /* Creo il file Camilla */
    if ((filecreato = creat("Camilla", PERM)) < 0)
    {
        printf("Errore nella creazione del file: 'Camilla'\n");
        exit(2);
    }
    
    /* Alloco memoria per l'array di pipe */
    piped = (pipe_t *)malloc(Q * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if (piped == NULL)
    {
        printf("Errore nella malloc\n");
        exit(3);
    }
    
    /* Creo le Q pipe */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(piped[q]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", q);
            exit(4);
        }
    }
    
    /* Creo i Q processi figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la creazione del processo figlio vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", q);
            exit(5);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo tutte le pipe che non servono */
            for (i = 0; i < Q; i++)
            {
                close(piped[i][0]);
                if (i != q)
                {
                    close(piped[i][1]);
                }
            }
            
            /* Apro il file associato al processo figlio */
            if ((fd = open(argv[q + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[q + 1]);
                exit(-1);
            }
            
            /* Setto a 0 i che fungera' da contatore */
            i = 0;

            /* Scorro tutti i caratteri del file */
            while (read(fd, &(linea[i]), 1))
            {
                if (linea[i] == '\n')
                {
                    /* Controllo che il primo carattere sia un numero e che la stringa sia lunga meno di 10 caratteri (compreso il terminatore) */
                    if ((isdigit(linea[0])) && (i + 1 < 10))
                    {
                        write(piped[q][1], linea, i + 1);
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
    /* Chiudo tutte le pipe che non servono */
    for (q = 0; q < Q; q++)
    {
        close(piped[q][1]);
    }
    
    /* Il padre recupera le informazioni dai figli */
    for (q = 0; q < Q; q++)
    {
        i = 0;
        /* Itero un ciclo che legge tutti i caratteri dalla pipe */
        while (read(piped[q][0], &(linea[i]), 1))
        {
            /* Controllo se sono arrivato alla fine di una linea */
            if (linea[i] == '\n')
            {
                linea[i + 1] = '\0';
                printf("Il processo figlio di indice q = %d ha trovato nel file %s la linea che soddisfa i requisiti: : %s", q, argv[q + 1], linea);
                write(filecreato, linea, i + 1);
                i = 0;
            }
            else
            {
                i++;
            }
        }
    }
    
    /* Il padre attende i figli */
    for (q = 0; q < Q; q++)
    {
        /* Controllo che la wait sia andata a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(6);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}