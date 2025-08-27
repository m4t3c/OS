#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define N 26
/* Definisco il tipo pipe_t */
typedef int pipe_t[2];

/* Definisco la struct */
typedef struct {
    char ch;        /* Carattere contato */
    long int occ;   /* Occorrenze del carattere contato */
} strut;

void bubbleSort(strut *v, int dim)
{
    int i;
    strut tmp;
    bool ordinato = false;
    while (dim > 0 && !ordinato)
    {
        ordinato = true; /* hp: è ordinato */
        for (i = 0; i < dim - 1; i++)
            if (v[i].occ > v[i + 1].occ)
            {
                tmp = v[i];
                v[i] = v[i + 1];
                v[i + 1] = tmp;
                ordinato = false;
            }
        dim--;
    }
}

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd;                         /* Per open del file */
    int pid[N];                     /* Array di 26 pid per fork */
    char c;                         /* Singolo carattere letto con la read dal file */
    int i, j;                       /* Indici per i cicli */
    pipe_t piped[N];                /* Array di pipe */
    strut s[N];                     /* Array di N struct strut */
    char C;                         /* Carattere da cercare in ogni filgio */
    long int curr_occ;              /* Numero di occorrenze in ogni processo figlio */
    int nr, nw;                     /* Per read e write */
    int pidFiglio, ritorno, status; /* Per wait */
    /* ------------------------------ */

    /* Controllo che sia passato un solo parametro */
    if (argc != 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di un parametro ma argc = %d\n", argc);
    }
    
    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la creazione della pipe vada a buon fine */
        if (pipe(piped[i]) < 0)
        {
            printf("Errore nella creazione della pipe numero %d\n", i);
            exit(2);
        }
        
    }
    
    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid[i] = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(3);
        }
        if (pid[i] == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe che non servono */
            for (j = 0; j < N; j++)
            {
                if (i != j)
                {
                    close(piped[j][1]);
                }
                if ((i == 0) || (j != i - 1))
                {
                    close(piped[j][0]);
                }
                
            }

            /* Apro in lettura il file passato come parametro */
            if ((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nella open di %s\n", argv[1]);
                exit(-1);
            }
            
            /* Inizializzo C e curr_occ */
            C = 'a' + i;
            curr_occ = 0L;

            /* Itero un ciclo che legge un carattere alla volta dal file */
            while (read(fd, &c, 1))
            {
                /* Controllo se il carattere letto è quello cercato */
                if (c == C)
                {
                    /* Incremento il numero di occorrenze */
                    curr_occ++;
                }
                
            }

            /* Se i != 0 leggo da pipe l'informazione mandata dal figlio precedente */
            if (i != 0)
            {
                /* Controllo che siano letti il numero di parametri letti */
                nr = read(piped[i - 1][0], &s, sizeof(s));
                if (nr != sizeof(s))
                {
                    printf("Errore nella lettura da pipe[%d]\n", i);
                    exit(-1);
                }
                
            }
            
            /* Aggiorno i dati della struct */
            s[i].ch = C;
            s[i].occ = curr_occ;
            
            /* Mando alla pipe successiva la struct aggiornata */
            nw = write(piped[i][1], &s, sizeof(s));
            if (nw != sizeof(s))
            {
                printf("Errore nella scrittura su pipe[%d]\n", i);
                exit(-1);
            }
            
            exit(c);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe inutilizzate */
    for (i = 0; i < N; i++)
    {
        /* Chiudo tutte le pipe in scrittura */
        close(piped[i][1]);
        /* Se i != da N - 1 chiudo anche in lettura */
        if (i != N - 1)
        {
            close(piped[i][1]);
        }
        
    }
    
    /* Recupero la struct dalla pipe */
    nr = read(piped[N - 1][0], &s, sizeof(s));
    if (nr != sizeof(s))
    {
        printf("Errore nella read da pipe[%d]\n", N - 1);
        exit(4);
    }
    
    /* Ordino la structr in senso crescente */
    bubbleSort(s, N);

    /* Stampo le informazioni per la struct ordinata */
    for (i = 0; i < N; i++)
    {
        printf("Il processo figlio di indice n = %d e pid %d ha trovato %ld occorrenze del carattere %c\n", s[i].ch - 'a', pid[s[i].ch - 'a'], s[i].occ, s[i].ch);
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < N; i++)
    {
        /* Controllo che la wait vada a buon fine */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %c (%d in decimale, se 255 problemi!)\n", pidFiglio, ritorno, ritorno);
        }
    }
    
    exit(0);
}