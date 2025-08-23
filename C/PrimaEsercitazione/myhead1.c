#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    
    /* ------ Variabili Locali ------ */
    int n;              /* Numero passato come parametro */
    int i;              /* Indice per il conteggio delle linee */
    char c;             /* Singolo carattere letto */
    /* ------------------------------ */

    /* Controllo che sia passato un solo parametro */
    if (argc != 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di un parametro ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Controllo che il parametro passato inizi con il carattere - */
    if (argv[1][0] != '-')
    {
        printf("Errore nel passaggio del parametro: %s non è stato passato nel formato giusto del tipo -n\n", argv[1]);
        exit(2);
    }
    
    /* Controllo che sia passato un numero intero strettamente positivo */
    if ((n = atoi(&(argv[1][1]))) <= 0)
    {
        printf("Errore nel passaggio del parametro: l'opzione non è corretta poichè n = %d\n", n);
        exit(3);
    }
    
    /* Inizializzo ad uno i che sta ad indicare che siamo nella prima riga */
    i = 1;

    /* Itero un ciclo che legge tutti i caratteri da standard input */
    while (read(0, &c, 1))
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