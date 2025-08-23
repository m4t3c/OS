#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

    /* ------ Variabili Locali ------ */
    int N;          /* Numero di parametri passati */
    int n;          /* Indice del ciclo for */
    /* ------------------------------- */

    /* Controllo che il numero di parametri passati sia almeno 1 */
    if (argc < 2)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 1 parametro ma argc = %d\n", argc);
        exit(1);
    }

    /* Salvo il numero di parametri passati nella variabile N */
    N = argc - 1;

    /* Stampo il nome dell'eseguibile e il numero di parametri passati su standard output */
    printf("A %s sono stati passati %d parametri\n", argv[0], N);

    /* Itero un ciclo che stampa tutti i parametri passati */
    for (n = 0; n < N; n++)
    {
        printf("Parametro numero %d: %s\n", n + 1, argv[n + 1]);
    }
    
    exit(0);
}