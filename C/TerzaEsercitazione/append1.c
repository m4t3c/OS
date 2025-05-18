/* Programma in C append.c che se esiste il file passato come parametro, lo apre in scrittura, altrimenti lo crea; quindi, scrive su tale file tutto quello che viene letto dallo standard input */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define PERM 0644	/* in ottale per diritti UNIX */

int main (int argc, char ** argv)
{  int outfile, nread;		/* usati per open/creat e valore ritorno read */		
   char buffer[BUFSIZ];		/* usato per i caratteri */

	if (argc != 2) /* controllo sul numero di parametri: deve essere esattamente 1! */
    	{	printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        	printf("Ci vuole 1 argomento: nome-file\n");
            	exit(1);
    	}

	/* se il numero di parametri e' giusto, andiamo a tentare l'apertura in scrittura del file passato come primo parametro */
	if ((outfile = open(argv[1], O_WRONLY | O_APPEND | O_CREAT , PERM)) < 0)
        {
		/* Stampo errore nel caso in cui fallisce sia l'apertura sia la creat */
		printf("Errore nella creazione file %s o nella sua apertura dato che outfile = %d\n", argv[1], outfile);
               	exit(2);
	}

	else 	/* se il file esiste, ci si posiziona alla fine */ 
		lseek(outfile, 0L, 2); 	
		/* NOTA BENE: si puo' usare invece che 2 la costante SEEK_END */
		/* 0L e' la costante 0 in formato LONG INT */

	/* leggiamo dallo standard input */
	while ((nread = read(0, buffer, BUFSIZ)) > 0)
	{	if (write(outfile, buffer, nread ) < nread) /* scriviamo sul file */
		{ exit(3); /* errore */ }
	} 	/* fine del file di input */

    	exit(0);
}
