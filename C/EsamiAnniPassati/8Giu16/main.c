#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define PERM 0644

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* Scrivo la funzione per la generazione di numeri casuali */
int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}

int main (int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                          /* Numero di file passati/processi figli */
    int H;                          /* Ultimo parametro */
    int pid;                        /* Per fork e wait */
    pipe_t *pipe_PF;                /* Array di pipe tra padre e figlio */
    pipe_t *pipe_FP;                /* Array di pipe tra figlio e padre */
    int i, j;                       /* Indici dei cicli */
    int fd;                         /* File descriptor per la open */
    int fout;                       /* File descriptor per il file creato */
    char linea[255];                /* Buffer per la linea letta */
    int r;                          /* Valore random generato dal padre */
    int ritorno = 0;                /* Variabile di ritorno dei processi figli e per wait */
    int valore;                     /* Valore letto nella read dal padre */
    int giusto;                     /* Lunghezza della linea su cui si generera' il numero random */
    int status;                     /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 5 parametri */
    if (argc < 6)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 5 caratteri (nomefile1, nomefile2, nomefile3, nomefile4, ..., H) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di file passati nella variabile N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un numero intero strettamente positivo e minore di 255 */
    H = atoi(argv[argc - 1]);
    if ((H <= 0) || (H >= 255))
    {
        printf("Errore nel passaggio dei parametri: %s non e' un numero intero strettamente positivo minore di 255\n", argv[argc - 1]);
        exit(2);
    }
    
    /* Inizializzo il seme per la generazione di numeri random */
    srand(time(NULL));

    /* Creo il file creato */
    if ((fout = open("/tmp/creato", O_CREAT | O_WRONLY | O_TRUNC, PERM)) < 0)
    {
        printf("Errore nella creazione del file /tmp/creato\n");
        exit(3);
    }
    

    /* Alloco memoria gli array di pipe */
    pipe_PF = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipe_FP = (pipe_t *)malloc(N * sizeof(pipe_t));
    /* Controllo che la malloc sia andata a buon fine */
    if ((pipe_PF == NULL) || (pipe_FP == NULL))
    {
        printf("Errore nella malloc dell'array di pipe\n");
        exit(4);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(pipe_PF[i]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra padre e figlio di indice i = %d\n", i);
            exit(5);
        }

        if (pipe(pipe_FP[i]) < 0)
        {
            printf("Errore nella creazione della pipe di comunicazione tra figlio e padre di indice i = %d\n", i);
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
            /* Si sceglie di ritornare -1 (255 senza segno) in caso di errore */
            /* Chiudo le pipe non necessarie */
            for (j = 0; j < N; j++)
            {
                close(pipe_FP[j][0]);
                close(pipe_PF[j][1]);
                if (j != i)
                {
                    close(pipe_FP[j][1]);
                    close(pipe_PF[j][0]);
                }
                
            }
            
            /* Apro in lettura il file associato */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura in lettura del file %s\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Inizializzo a 0 j che fungera' da indice di linea */
            j = 0;
            /* Itero un ciclo che scorre tutti i caratteri del file */
            while (read(fd, &(linea[j]), 1))
            {
                /* Controllo se sono arrivato alla fine della linea */
                if (linea[j] == '\n')
                {
                    /* Aggiungo il terminatore e invio al padre la lunghezza della linea */
                    j++;
                    write(pipe_FP[i][1], &j, sizeof(j));
                    /* Ricevo dal padre il valore random generato */
                    read(pipe_PF[i][0], &r, sizeof(r));
                    /* Controllo se l'indice passato e' ammissibile per la linea corrente */
                    if (r < j)
                    {
                        write(fout, &(linea[r]), 1);
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
    /* Chiudo le pipe inutilizzate */
    for (i = 0; i < N; i++)
    {
        close(pipe_PF[i][0]);
        close(pipe_FP[i][1]);
    }
    
    /* Il padre recupera le informazioni dai figli e invia l'indice random */
    for (j = 0; j < H; j++)
    {
        /* Setto la variabile r con un numero random che corrispondera' al processo figlio da cui prendere la lunghezza della linea */
        r = mia_random(N);
        for (i = 0; i < N; i++)
        {
            read(pipe_FP[i][0], &valore, sizeof(valore));
            if (i == r)
            {
                giusto = valore;
            }
        }
        
        /* Genero l'indice casuale */
        r = mia_random(giusto);
        /* Invio ai processi figli l'indice generato */
        for (i = 0; i < N; i++)
        {
            write(pipe_PF[i][1], &r, sizeof(r));
        }
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}