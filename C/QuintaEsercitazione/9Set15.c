#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

/* Definisco il tipo pipe_t come array di 2 int */
typedef int pipe_t[2];

void handler(int signo)
{
    printf("DEBUG-Sono il padre %d e ho ricevuto il segnale %d\n", getpid(), signo);
}

int main (int argc, char **argv)
{

    /* ------ Variabili locali ------ */
    int N;                  /* Numero di file passati per i processi figli */
    int *pid;               /* Array di pid per la fork */
    /* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno tre parametri (nomi di file) ma argc = %d\n", argc);
        exit(1);
    }

    /* Assegno ad N il numero di parametri passati escluso l'ultimo */
    N = argc - 2;
    
    /* Installo il gestore handler per il segnale SIGPIPE */
    signal(SIGPIPE, handler);

    /* Alloco memoria per l'array di pid */
    pid = (int *)malloc(N * sizeof(int));
    /* Controllo che l'allocazione sia andata a buon fine */
    if(pid == NULL)
    {
        printf("Errore nalla malloc dell'array di pid\n");
        exit(2);
    }

    
}