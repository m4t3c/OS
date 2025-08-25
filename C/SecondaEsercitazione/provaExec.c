#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {

    /* ------ Variabili Locali ------ */
    int n;                      /* Numero letto con la scanf */
    char *argin[2];             /* Array che contiene il nome dell'eseguibile e il terminatore */
    /* ------------------------------ */

    /* Inizializzo argin */
    argin[0] = "./provaExec";
    argin[1] = (char *)0;

    /* Stampo che sto eseguendo ./provaExec */
    printf("Sto eseguendo %s\n", argin[0]);

    /* Itero un ciclo che continua finchè il numero letto da standard input è 1 */
    printf("Vuoi invocare ricorsivamente %s?\n", argin[0]);
    scanf("%d", &n);
    /* Controllo il valore di n */
    if (n != 0)
    {
        execv(argin[0], argin);
        printf("Errore nella execv\n");
        exit(1);
    }
    
    exit(n);
}