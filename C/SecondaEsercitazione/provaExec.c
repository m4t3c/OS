#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	char *argin[2];		/* Arrai che contiene il nome dell'eseguibile ed il terminatore */
	int x;			/* Variabile per salvare il valore dell'utente */

	argin[0] = "./provaExec";	/* Inizializzo il primo parametro con il nome dell'eseguibile */
	argin[1] = (char *)0;		/* Inizializzo il secondo parametro con il terminatore */

	printf("Eseguo %s\n", argin[0]);
	printf("Vuoi continuare con l'esecuzione? (0 se vuoi terminare)\n");
	scanf("%d", &x);
	if(x != 0)	/* Caso in cui l'utente voglia continuare con l'esecuzione ricorsiva */
	{
		execv(argin[0], argin);
		printf("Errore nella execv\n");		/*Comando che verra' eseguito solo in caso di errore */
		exit(1);
	}

	exit(x);	/* Quando si arrivera' a questa parte di programma, x varra' 0 */
}
