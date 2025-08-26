#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;                          /* Numero di parametri passati */
    int n;                          /* Indice per i processi figli */
    int fd;                         /* Per open */
    int pid;                        /* Per fork */
    int p[2][2];                    /* Array di due array di due interi */
    char ch, ch0, ch1;              /* Singolo carattere letto */
    int nr0, nr1;                   /* Numero di caratteri alfabetici e numerici inviati dai figli */
    int totale = 0;                 /* Contatore dei caratteri */
    int pidFiglio, status, ritorno; /* Per wait */
    /* ------------------------------- */

    /* Controllo che siano passati almeno 2 parametri */
    if (argc < 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di parametri passati in N */
    N = argc - 1;

    /* Creo le due pipe */
    if (pipe(p[0]) < 0)
    {
        printf("Errore nella creazione della pipe per i figli di indice dispari\n");
        exit(2);
    }

    if (pipe(p[1]) < 0)
    {
        printf("Errore nella creazione della pipe per i figli di indice pari\n");
        exit(3);
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
            /* Chiudo la pipe che non userò */
            close(p[0][0]);
            close(p[1][0]);
            close(p[n % 2][1]);

            /* Apro in lettura il file associato al processo figlio */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file %s\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Itero un ciclo che legge tutti i caratteri del file */
            while (read(fd, &ch, 1) > 0)
            {
                /* Controllo se sono in uno dei casi desiderati */
                if (((n % 2) == 0 && isalpha(ch)) || ((n % 2) == 1 && isdigit(ch)))
                {
                    write(p[(n + 1) % 2][1], &ch, 1);

                }
                
            }
            
            exit(ch);
        }
        
    }
    
    /* Processo padre */
    /* Chiudo le pipe di cui non ho bisogno */
    close(p[0][1]);
    close(p[1][1]);

    /* Faccio una stampa preliminare per annunciare che sto per stampare i caratteri */
    printf("I caratteri inviati dai figli sono:\n");

    /* Leggo un singolo carattere da entrambe le pipe */
    nr0 = read(p[0][0], &ch0, 1);
    nr1 = read(p[1][0], &ch1, 1);

    /* Itero un ciclo finchè ci sono caratteri da leggere */
    while ((nr0 != 0) || (nr1 != 0))
    {
        /* Incremento il numero totale di caratteri letti */
        totale += nr0 + nr1;
        /* Scrivo su standard output i caratteri letti */
        write(1, &ch1, nr1);
        write(1, &ch0, nr0);

        /* Provo a leggere altri caratteri dalle pipe */
        nr0 = read(p[0][0], &ch0, 1);
        nr1 = read(p[1][0], &ch1, 1);
    }
    
    /* Stampo il numero di caratteri scritti su standard output */
    printf("\nNumero di caratteri scritti su standard output: %d\n", totale);

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
            printf("Processo filgio con PID: %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %c ( in decimale %d, se 255 problemi!)\n", pidFiglio, ritorno, ritorno);
        }
    }
    
    exit(0);
}