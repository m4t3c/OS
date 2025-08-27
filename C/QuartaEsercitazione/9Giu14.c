#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Defiisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di parametri passati */
    int n;                          /* Indice dei processi figli */
    int i;                          /* Indice */
    int pid;                        /* Per fork */
    int fd;                         /* Per open */
    pipe_t *piped;                  /* Array di pipe per padre-figli */
    int p[2];                       /* Pipe tra figlio e nipote */
    long int sum = 0L;              /* Somma dei valori ricevuti */
    char numero[11];                /* Array di caratteri dove memorizzare la stringa corrisp al numero */
    int valore;                     /* Stringa convertita in numero */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------ */
    
    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo in N il numero di parametri passati */
    N = argc - 1;

    /* Alloco memoria per un array di N pipe */
    if ((piped = (pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella malloc dell'array di pipe");
        exit(2);
    }
    
    /* Creo le N pipe tra padre e figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la creazione della pipe abbia successo */
        if (pipe(piped[n]) < 0)
        {
            printf("Errore nella creazione della pipe di indice n = %d\n", n);
            exit(3);
        }
        
    }
    
    /* Creo gli N processi figli */
    for (n = 0; n < N; n++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice n = %d\n", n);
            exit(4);
        }
        if (pid == 0)
        {
            /* Processo figlio */
            /* Chiudo le pipe non utilizzate */
            for (i = 0; i < N; i++)
            {
                close(piped[i][0]);
                if (i != n)
                {
                    close(piped[i][1]);
                }
                
            }
            
            /* Creo la pipe tra figlio e nipote */
            if (pipe(p) < 0)
            {
                printf("Errore nella creazione della pipe tra figlio e nipote\n");
                exit(-1);
            }
            
            /* Controllo che la fork per il processo nipote vada a buon fine */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella fork per il processo nipote\n");
                exit(-1);
            }
            if (pid == 0)
            {
                /* Processo nipote */
                /* Chiudo le pipe non utilizzate */
                close(piped[n][1]);

                /* Chiudo lo standard input e lo redireziono sul file passato come parametro */
                close(0);
                if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nella open del file %s\n", argv[n + 1]);
                    exit(-1);
                }
                
                /* Chiudo lo standard output e lo redireziono su p[1] */
                close(1);
                dup(p[1]);
                /* Chiudo le restanti pipe */
                close(p[0]);
                close(p[1]);

                /* Redireziono lo standard error su /dev/null */
                close(2);
                open("/dev/null", O_WRONLY);

                /* Eseguo wc -l */
                execlp("wc", "wc", "-l", (char *)0);

                /* Non si dovrebbe arrivare quì */
                exit(-1);
            }
            
            /* Processo figlio */
            /* Chiudo la pipe non utilizzata */
            close(p[1]);

            /* Inizializzo i a 0 */
            i = 0;

            /* Leggo dalla pipe un carattere alla volta finchè non arrivo al carattere '\n' */
            while (read(p[0], &numero[i], 1))
            {
                i++;
            }
            
            /* Se il filgio ha letto qualcosa trasformo il valore in stringa */
            if (i != 0)
            {
                numero[i - 1] = '\0';
                valore = atoi(numero);
            }
            else {
                valore = 0;
            }
            
            /* Il figlio comunica al padre il valore */
            write(piped[n][1], &valore, sizeof(valore));

            /* Il figlio aspetta il nipote */
            /* Se il nipote termina in modo anomalo deve ritornare -1 */
            ritorno = -1;
            if ((pidFiglio = wait(&status)) < 0)
            {
                printf("Errore nella wait del processo nipote\n");
            }
            else if ((status & 0xFF) != 0)
            {
                printf("Nipote con PID: %d temrinato in modo anomalo\n", pidFiglio);
            }
            else
            {
                ritorno = (int)((status >> 8) & 0xFF);
            }
            
            exit(ritorno);
        }
        
    }
    
    /* Processo padre */
    /* Il padre chiude le pipe che non usa */
    for (n = 0; n < N; n++)
    {
        close(piped[n][1]);
    }
    
    /* Il padre recupera le informazioni dai figli */
    for (n = 0; n < N; n++)
    {
        /* Leggo da pipe i valori */
        read(piped[n][0], &valore, sizeof(valore));
        sum += (long int)valore;
    }
    
    /* Stampo il numero di righe totali */
    printf("Sono state contate in totale %ld righe\n", sum);

    /* Il padre aspetta i figli */
    for (n = 0; n < N; n++)
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
            printf("Il processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}