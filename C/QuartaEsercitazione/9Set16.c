#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define N 26                                    /* Numero di figli da creare */
typedef int pipe_t[2];                          /* Definisco un array di 2 interi che mi servira' come array di pipe */

typedef struct
{
    char ch;
    long int occ;
} Data;

void bubbleSort(Data *v, int dim) {
   int i;
   Data tmp;
   int ordinato = 0;
	while(dim > 0 && !ordinato)
	{
		ordinato = 1;
		for(i=0; i < dim-1; i++)
		{
			if(v[i].occ > v[i+1].occ)
			{
				tmp = v[i];
				v[i] = v[i+1];
				v[i+1] = tmp;
				ordinato = 0;
			}
		}
		dim--;
	}
}

int main(int argc, char ** argv)
{
    int pid[N];                                    /* Per fork */
    int fd;                                     /* File descriptor per la open */
    long int curr_occ;                          /* Counter delle occorrenze di ogni carattere */
    pipe_t piped[N];                            /* Array di 26 pipe usate nei processi figli */
    Data d[N];                                  /* Struct per i 26 caratteri da leggere */ 
    int i, j;                                   /* Indici per i cicli */
    char C;                                     /* Carattere di cui bisogna contare le occorrenze nell'i-esimo figlio */
    char c;                                     /* Carattere letto dai figli */
    int nr, nw;                                 /* Numero parametri letti e scritti */
    int pidFiglio, status, ritorno;             /* Per wait */

    if(argc != 2)                               /* Controllo che il numero di parametri passati sia corretto */
    {
        printf("Errore nel numero di parametri: ho bisogno di 1 parametro (nome assoluto di file) ma argc = %d\n", argc);
        exit(1);
    }

    /* Creo le N pipe */
    for (i = 0; i < N; i++)
    {
        if((pipe(piped[i])) < 0)                /* Controllo che la creazione della i-esima pipe vada a buon fine */
        {
            printf("Errore nella creazione della pipe di indice i = %d\n", i);
            exit(2);
        }
    }

    /* Creo gli N processi figli */
    for (i = 0; i < N; i++)
    {
        if((pid[i] = fork()) < 0)                  /* Controllo che l'i-esimo processo figlio sia creato con successo */
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(3);
        }
        if(pid[i] == 0)
        {
            /* Processo figlio */
            C = 'a' + i;

            /* Chiudo tutti i lati non usati delle pipe */
            for (j = 0; j < N; j++)
            {

                /* Nel caso in cui gli indici siano diversi allora devo chiudere la pipe in scrittura
                   Invece nel caso siamo nel primo processo figlio oppure che non siamo nel processo figlio immediatamente precedente a quello creato dobbiamo chiudere le pipe di lettura */
                if(i != j)
                {
                    close(piped[j][1]);
                }
                else if((i == 0) || (j != i - 1))
                {
                    close(piped[j][0]);
                }
            }

            if ((fd = open(argv[1], O_RDONLY)) < 0)     /* Controllo che la open vada a buon fine */
            {
                printf("Errore: %s non file o non apribile\n", argv[1]);
                exit(-1);
            }

            curr_occ = 0L;

            while (read(fd, &c, 1))
            {
                if(c == C)                      /* Controllo se il carattere letto e' quello cercato */
                {
                    curr_occ++;
                }
            }
            
            if(i != 0)                          /* Nel caso in cui non cerco il primo carattere leggo dalla pipe del filgio precedente l'array di structure */
            {
                nr = read(piped[i - 1][0], d, sizeof(d));
                
                if(nr != sizeof(d))             /* Controllo che la lettura sia andata a buon fine */
                {
                    printf("Errore in lettura da pipe[%d]\n", i - 1);
                    exit(-1);
                }
            }

            /* Inizializzo la struct con le nuove informazioni */
            d[i].ch = C;
            d[i].occ = curr_occ;

            nw = write(piped[i][1], d, sizeof(d));
            
            if(nw != sizeof(d))                 /* Controllo che la scrittura sia andata a buon fine */
            {
                printf("Errore in scrittura da pipe[%d]\n", i);
                exit(-1);
            }
            
            exit(c);
        }
    }
    
    /* Processo padre */
    
    /* Chiudo tutte le pipe che non servono */
    for (int i = 0; i < N; i++)
    {
        close(piped[i][1]);
        if(i != N - 1)
        {
            close(piped[i][0]);
        }
    }

    /* Leggo l'ultima struct dell'array d */
    nr = read(piped[N - 1][0], d, sizeof(d));
    
    if(nr != sizeof(d))
    {
        printf("Errore in lettura della pipe[N - 1] per il padre\n");
        exit(4);
    }
    else
    {
        /* Ordino l'array ricevuto */
        bubbleSort(d, N);
        /* Stampo i contenuti della struttura */
        for (i = 0; i < N; i++)
        {
            printf("Il processo figlio di indice: %d ha trovato %ld occorrenze del carattere %c nel file %s\n", i, d[i].occ, d[i].ch, argv[1]);
        }
    }

    /* Itero un ciclo per aspettare i processi figli */
    for (i = 0; i < N; i++)
    {
        if((pidFiglio = wait(&status)) < 0)     /* Controllo che la wait vada a buon fine */
        {
            printf("Errore nella wait\n");
            exit(5);
        }
        if((status & 0xFF) != 0)
        {
            printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d e' terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}