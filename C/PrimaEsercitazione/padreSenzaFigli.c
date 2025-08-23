#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {

    /* ------ Variabili Locali ------ */
    int pidFiglio, status, ritorno;          /* Per la wait */
    /* ------------------------------ */

    printf("Sono il processo padre e ho PID: %d\n", getpid());

    /* Il padre aspetta il filgio */
    if ((pidFiglio = wait(&status)) != 0)
    {
        printf("Errore nella wait\n");
        exit(1);
    }
    if ((status & 0xFF) != 0)
    {
        printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
    } else {
        ritorno = (int)((status >> 8) & 0xFF);
        printf("Processo figlio con PID: %d ha ritornato %d\n", pidFiglio, ritorno);
    }
    
    exit(0);
}