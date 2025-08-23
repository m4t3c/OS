#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    
    /* ------ Variabili Locali ------ */
    int fd = 0;         /* File su cui applicare il comando filtro head, di default su standard input */
    int n;              /* Numero di linee da scrivere su standard output */
    int i;              /* Indice per il conteggio delle linee */
    char c;             /* Singolo carattere letto */
    /* ------------------------------ */

    /* Controllo che siano passati massimo 2 parametri */
    if (argc > 3)
    {
        printf("Errore nel numero dei parametri: ho bisogno di massimo due parametri ma argc = %d\n", argc);
        exit(1);
    }

    /* Se sono passati 2 parametri apro in lettura il file passato come secondo parametro */
    if (argc == 3)
    {
        /* Controllo che il file sia apribile in lettura */
        if ((fd = open(argv[2], O_RDONLY)) <= 0)
        {
            printf("Errore nel passaggio dei parametri: %s non è un file o non è apribile in lettura\n", argv[2]);
            exit(2);
        }
    
    }
    
    
    /* Nel caso in cui sia passato almeno un parametro faccio i controlli su di esso in caso contrario inizializzo n a 10 */
    if (argc >= 2)
    {
        /* Controllo che il parametro passato inizi con il carattere - */
        if (argv[1][0] != '-')
        {
            printf("Errore nel passaggio del parametro: %s non è stato passato nel formato giusto del tipo -n\n", argv[1]);
            exit(3);
        }
        
        /* Controllo che sia passato un numero intero strettamente positivo */
        if ((n = atoi(&(argv[1][1]))) <= 0)
        {
            printf("Errore nel passaggio del parametro: l'opzione non è corretta poichè n = %d\n", n);
            exit(4);
        }
    } else {
        n = 10;
    }
    
    /* Inizializzo ad uno i che sta ad indicare che siamo nella prima riga */
    i = 1;

    /* Itero un ciclo che legge tutti i caratteri da standard input */
    while (read(fd, &c, 1))
    {
        /* Controllo se il carattere letto corrisponde a \n in tal caso incremento i */
        if (c == '\n')
        {
            i++;
        }
        
        /* Scrivo su standard output ogni carattere letto */
        write(1, &c, 1);

        /* Se i > n interrompo il ciclo */
        if (i > n)
        {
            break;
        }

    }
    
    exit(0);
}