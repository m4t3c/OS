/* Programma che accetta 2 parametri che sono nomi di file e li apre in lettura poi chiude il primo e crea una pipe e infine si stampano i valori dei due file descriptor della pipe */

#include <stdio.h> 	/* per poter usare printf e altre funzioni di I/O */
#include <stdlib.h>	/* per poter usare la primitiva exit */
#include <unistd.h>	/* per poter usare la primitiva close */
#include <fcntl.h>	/* per poter usare le costanti per la open (O_RDONLY, O_WRONLY e O_RDWR)*/

int main(int argc, char **argv)
{
	/*-----Variabili-----*/
	int fd1, fd2;		/* Variabili per salvare il valore di ritorno della open */
	int piped[2];		/* Array di due interi per la pipe */
	/*-------------------*/

	if(argc != 3)		/* Controllo che siano passati due parametri */
	{
		printf("Errore nel numero dei parametri: ho bisogno di due parametri (nomi di file) ma argc = %d\n", argc);
		exit(1);
	}
	
	if((fd1 = open(argv[1], O_RDONLY)) < 0)		/* Controllo che l'apertura del primo parametro vada a buon fine */
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file o non e' apribile in lettura\n", argv[1]);
		exit(2);
	}

	printf("Valore di fd1 = %d\n", fd1);

	if((fd2 = open(argv[2], O_RDONLY)) < 0)		/* Controllo che l'apertura del secondo parametro vada a buon fine */
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file o non e' apribile in lettura\n", argv[2]);
		exit(3);
	}

	printf("Valore di fd2 = %d\n", fd2);

	close(fd1);					/* Chiudo il file passato come primo parametro */

	if(pipe(piped) < 0)				/* Controllo che la creazione della pipe vada a buon fine */
	{
		printf("Errore nella creazione della pipe\n");
		exit(4);
	}

	printf("Creata pipe dal lato lettura piped[0] = %d\n", piped[0]);
	printf("Creata pipe dal lato scrittura piped[1] = %d\n", piped[1]);

	exit(0);

}
