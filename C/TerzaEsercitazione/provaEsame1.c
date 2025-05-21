/* Programma in C che accetta 2 o più parametri che rappresentano nomi di file. In seguito genera N processi figli ognuno dei quali selleziona per gli N pari i caratteri numerici metre per gli N dispari i caratteri alfabetici del file passato come parametro. Il processo padre scriverà su standard output un carattere alfabetico e uno numerico, finche' l'alternanza sara' possibile, su standard output. Ogni processo figlio ritorna l'ultimo carattere letto. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

int main(int argc, char **argv)
{
	/*------Variabili------*/
	int p[2][2];			/* Creo due pipe p[0] per i processi dispari e p[1] per i processi pari */
	int N;				/* Numero di file */
	int pid;			/* Ritorno della fork */
	int fdr;			/* File descriptor per aprire ogni file */
	int n;				/* indice processi */
	int tot = 0;			/* Numero totale di caratteri scritti su standard output */
	int nr0, nr1;			/* nr0 conta il numero di caratteri su p[0] nr1 conta il numero di caratteri su p[1] */
	char ch, ch0, ch1;		/* ch usato da ogni figlio per leggere il singolo carattere da file, ch0 per leggere da p0 e ch1 per leggere da p1 */
	int pidFiglio, status, ritorno;	/* Per wait */
	
	if(argc < 3)			/* Controllo che siano passati almeno due parametri */
	{
		printf("Errore nel numero dei parametri: %s ha bisogno di almeno 2 parametri (nomi di file) ma argc = %d\n", argv[0], argc);
		exit(1);
	}

	N = argc - 1;

	if(pipe(p[0]) < 0)		/* Controllo che la creazione di p[0] vada a buon fine */
	{
		printf("Errore nella creazione della prima pipe\n");
		exit(2);
	}

	if(pipe(p[1]) < 0)		/* Controllo che la creazione di p[1] vada a buon fine */
	{
		printf("Errore nella creazione della seconda pipe\n");
		exit(3);
	}

	for(n = 0; n < N; ++n)		/* Inizializzo un ciclo per creare gli N processi figli */
	{
		if((pid = fork()) < 0)	/* Controllo che la creazione dell'n-esimo processo figlio vada a buon fine */
		{
			printf("Errore nella fork\n");
			exit(4);
		}
		if(pid == 0)
		{
			/* Processo figlio */
			/* Chiudo le pipe in lettura di entrambe e chiudo quella in scrittura della pipe non usata */
			close(p[0][0]);
			close(p[1][0]);
			close(p[n % 2][1]);
			if((fdr = open(argv[n + 1], O_RDONLY)) < 0)	/* Controllo che l'apertura del file abbia successo */ 
			{
				printf("Errore nel passaggio dei file: %s non file o non apribile\n", argv[n + 1]);
				exit(-1);
			}

			while(read(fdr, &ch, 1) > 0)	/* Itero un ciclo che legge un carattere alla volta finche' ne ha la possibilita' */
			{
				if((((n % 2) == 0) && isalpha(ch)) || ((((n % 2) == 1) && isdigit(ch))))		/* Controllo che nel caso sia un parametro pari sia un numero il carattere letto e nel caso sia un parametro dispari sia una lettera */
				{
					write(p[(n + 1) % 2][1], &ch, 1);	/* Comunica il carattere al padre */
				}
			}

			exit(ch);		/* Ritorno l'ultimo carattere letto */
		}
	}

	/* Processo padre */
	/* Chiudo entrambe le pipe in scrittura */
	close(p[0][1]);
	close(p[1][1]);

	printf("I caratteri inviati dai processi figli sono:\n");
	
	nr0 = read(p[0][0], &ch0, 1);		/* Leggo il primo carattere da p[0] */
	nr1 = read(p[1][0], &ch1, 1);		/* Leggo il primo carattere da p[1] */
	
	while((nr0 != 0) || (nr1 != 0))		/* Inizializzo un ciclo che va avanti finche' ci sono caratteri da leggere */
	{
		tot += (nr0 + nr1);		/* Aggiorno il numero di caratteri letti */

		write(1, &ch0, 1);		/* Scrivo un carattere alfabetico */
		write(1, &ch1, 1);		/* Scrivo un carattere numerico */

		nr0 = read(p[0][0], &ch0, 1);
		nr1 = read(p[1][0], &ch1, 1);
	}

	printf("\nIl numero di caratteri letti e': %d\n", tot);
	
	for(n = 0; n < N; ++n)
	{
		if((pidFiglio = wait(&status)) < 0)	/* Controllo che la wait abbia esito positivo */
		{
			printf("Errore nella wait\n");
			exit(5);
		}
		if((status & 0xFF) != 0)
		{
			printf("Processo figlio con PID = %d terminato in modo anomalo\n", pidFiglio);
		}
		else
		{
			ritorno =(int)((status >> 8) * 0xFF);
			printf("Il processo figlio con PID = %d e' terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
		}
	}
	
	exit(0);
}
