#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------- */
    int fd;                             /* Per open */
    int pid;                            /* Per fork */
    int pidFiglio, status, ritorno;     /* Per wait */
    /* ------------------------------- */

    /* Controllo che sia passato esattamente un parametro */
    if (argc != 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di esattamente un parametro ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che la fork vada a buon fine */
    if ((pid = fork()) < 0)
    {
        printf("Errore nella fork\n");
        exit(2);
    }
    if (pid == 0)
    {
        /* Processo filgio */
        /* Redireziono standard input sul file passato come parametro */
        close(0);
        if ((fd = open(argv[1], O_RDONLY)) < 0)
        {
            printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
            exit(-1);
        }
        
        /* Eseguo il programma mycat */
		execlp("./mycat", "./mycat", (char *)0);

        printf("Errore nella execlp\n");
        exit(-1);
    }

    /* Processo padre */
    /* Il padre aspetta il figlio */
    if ((pidFiglio = wait(&status)) < 0)
    {
        printf("Errore nella wait\n");
        exit(3);
    }
    if ((status & 0xFF) != 0)
    {
        printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
    }
    else
    {
        ritorno = (int)((status >> 8) & 0xFF);
        printf("Processo figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
    }
    
    exit(0);
}