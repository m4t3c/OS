/* Programma in C provaPipe.c che ha bisogno di un singolo parametro che deve essere un file con uno specifico formato: numero non noto di linee (ma strettamente minore di 255) e ogni linea composta da 4 caratteri (oltre il terminatore di linea '\n'); il processo padre genera un processo figlio che legge via via le linee dal file; per ogni linea letta, la trasforma in stringa e la invia al padre che la riporta su standard output */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int pid, j, piped[2]; /* pid per fork, j per contatore, piped per pipe */
    char mess[255];       /* array usato dal figlio per inviare la stringa al padre e array usato dal padre per ricevere la stringa inviata dal figlio */
    /* N.B. dato che ogni processo (padre e figlio) avra' la propria AREA DATI il contenuto di questo array sara' PRIVATO di ogni processo e quindi NON serve creare due array distinti per il padre e per il figlio! */
    /* La stessa cosa vale anche per tutte le altre variabili chiaramente! */
    int i;                          /* Indice per la lettura di un singolo carattere */
    int fd;                         /* variabile usata dal figlio per open file */
    int pidFiglio, status, ritorno; /* per wait padre */

    /* controllo sul numero di parametri: esattamente uno */
    if (argc != 2)
    {
        printf("Errore numero di parametri: %s vuole un singolo parametro (nomeFile) e invece argc = %d\n", argv[0], argc);
        exit(1);
    }

    /* si crea una pipe: si DEVE sempre controllare che la creazione abbia successo!  */
    if (pipe(piped) < 0)
    {
        printf("Errore creazione pipe\n");
        exit(2);
    }

    /* si crea un figlio (controllando sempre!) */
    if ((pid = fork()) < 0)
    {
        printf("Errore creazione figlio\n");
        exit(3);
    }

    if (pid == 0)
    {
        /* figlio */
        close(piped[0]);                        /* il figlio CHIUDE il lato di lettura: sara' quindi lo SCRITTORE della pipe */
        if ((fd = open(argv[1], O_RDONLY)) < 0) /* apriamo il file */
        {
            printf("Errore in apertura file %s\n", argv[1]);
            exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
        }

        printf("DEBUG-Figlio %d sta per iniziare a scrivere una serie di messaggi, ognuno di lunghezza generica, sulla pipe dopo averli letti dal file passato come parametro\n", getpid());
        
        /* Setto i contatori a 0 */
        i = 0; 
        j = 0;

        /* il figlio legge tutto il file passato come parametro */
        while (read(fd, &(mess[i]), 1) != 0) /* il contenuto del file e' tale che in mess ci saranno 4 caratteri e il terminatore di linea */
        {
            if (mess[i] == '\n')
            {
                /* Invio al padre il numero di caratteri e la linea letta */
                mess[i + 1] = '\0';
                i++;
                write(piped[1], &i, sizeof(int));
                write(piped[1], mess, i);

                /* Risetto i a 0 e incremento il numero di messaggi */
                i = 0;
                j = 0;
            }
            else
            {
                i++;
            }
            
        }
        printf("DEBUG-Figlio %d scritto %d messaggi sulla pipe\n", getpid(), j);
        exit(j); /* figlio deve tornare al padre il numero di linee lette che corrisponde al numero di stringhe mandate al padre, supposto < 255! */
    }

    /* padre */
    close(piped[1]); /* il padre CHIUDE il lato di scrittura: sara' quindi il LETTORE della pipe */
    printf("DEBUG-Padre %d sta per iniziare a leggere i messaggi dalla pipe\n", getpid());
    j = 0;                                /* il padre inizializza la sua variabile j per verificare quanti messaggi gli ha mandato il figlio */
    while (read(piped[0], &i, sizeof(i))) /* questo ciclo avra' termine appena il figlio terminera' dato che la read senza piu' scrittore tornera' 0! */
    {
        /* Leggo dal figlio la stringa effettiva */
        read(piped[0], mess, i);
        
        /* Stampo la stringa letta con il numero associato */
        printf("%d: %s\n", j, mess);
        j++; /* incrementiamo il contatore di messaggi */
    }
    printf("DEBUG-Padre %d letto %d messaggi dalla pipe\n", getpid(), j);

    /* padre aspetta il figlio */
    if ((pidFiglio = wait(&status)) < 0)
    {
        printf("Errore wait\n");
        exit(4);
    }
    if ((status & 0xFF) != 0)
        printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
    else
    {
        ritorno = (int)((status >> 8) & 0xFF);
        printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
    }

    exit(0);
}