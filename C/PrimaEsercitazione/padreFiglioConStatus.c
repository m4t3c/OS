#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

/* Funzione mia_random per generare numeri casuali */
int mia_random(int n) {
    int casuale;
    casuale = rand() % n;
    return casuale;
}

int main() {

    /* ------ Variabili Locali ------ */
    int pid;            /* Per valore di ritorno della fork */
    int pidFiglio;      /* Per valore di ritorno della wait */
    int status;         /* Per wait */
    int ritorno;        /* Valore di uscita processo figlio */
    int r;              /* Valore random generato dal processo figlio */
    /* ------------------------------ */

    /* Stampo il PID del processo padre */
    printf("Sono il processo padre e ho PID: %d\n", getpid());

    /* Inizializzo il seme della funzione mia_random */
    srand(time(NULL));

    /* Controllo che la fork abbia successo */
    if ((pid = fork()) < 0)
    {
        printf("Errore nella fork\n");
        exit(1);
    }

    if (pid == 0)
    {
    
        /* Processo figlio */

        printf("Sono il processo figlio del processo padre con PID: %d e ho PID: %d\n", getppid(), getpid());

        /* Genero il numero casuale e lo ritorno in uscita */
        r = mia_random(100);
        exit(r);
    }

    /* Processo padre */
    /* Il padre aspetta il processo figlio */
    if ((pidFiglio = wait(&status)) < 0)
    {
        printf("Errore nella wait %d\n", pidFiglio);
        exit(2);
    } 
    if ((status & 0xFF) != 0)
    {
        printf("Figlio terminato in modo anomalo\n");
    } else {

        ritorno = (int)((status >> 8) & 0xFF);
        printf("Il figlio con PID: %d ha ritornato %d\n", pidFiglio, ritorno); 
    }
    
    exit(0);
}