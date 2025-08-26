#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int fd1, fd2;                   /* Per open del primo e secondo parametro */
    int piped[2];                   /* Array di due interi per la pipe */
    /* ------------------------------ */

    /* Controllo che siano passati 2 parametri */
    if (argc != 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di esattamente 2 parametri ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che il primo file sia aperto correttamente */
    if ((fd1 = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[1]);
        exit(2);
    }
    
    /* Stampo il valore di fd1 */
    printf("Valore di fd1 = %d\n", fd1);

    /* Controllo che il secondo file sia aperto correttamente */
    if ((fd2 = open(argv[2], O_RDONLY)) < 0)
    {
        printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[2]);
        exit(3);
    }
    
    /* Stampo il valore di fd2 */
    printf("Valore di fd2 = %d\n", fd2);

    close(fd1);

    /* Controllo che la pipe sia creata correttamente */
    if (pipe(piped) < 0)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }
    
    printf("Il valore della pipe in letuttura è: %d\n", piped[0]);
    printf("Il valore della pipe in scrittura è: %d\n", piped[1]);

    exit(0);
}