#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];			/* Definisco il tipo pipe_t come array di 2 int */

int main(int argc, char **argv)
{
	int pid;			/* Per fork */
	int N;				/* Numero di parametri passati */
	int i, j;			/* Indici */
	int valore;			/* Numero di linee di un file */
	long int somma = 0;		/* Valore che somma tutte le linee */
	pipe_t *piped;			/* Array dinamico di pipe */
	pipe_t p;			/* Pipe di comunicazione tra figlio e nipote */
	char numero[11];		/* Array che contiene la lunghezza in righe di un file */
	int pidFiglio, status, ritorno;	/* Per wait */
	
	if(argc < 3)			/* Controllo che siano passati almeno 2 parametri */
	{
		printf("Errore nel numero dei parametri: ho bisogno di almeno 2 parametri (nomi assoluti di file) ma argc = %d\n", argc);
		exit(1);
	}
	
	N = argc - 1;
	
	piped = (pipe_t *) malloc(N * sizeof(pipe_t));	/* Alloco memoria per N pipe */
	
	if(piped == NULL)		/* Controllo che l'allocazione di memoria per l'array di pipe sia andata a buon fine */
	{
		printf("Errore nell'allocazione della memoria\n");
		exit(2);
	}

	for (i = 0; i < N; i++)		/* Itero un ciclo per inizializzare le N pipe di comunicazione tra padre e figlio */
	{
		if(pipe(piped[i]) < 0)	/* Controllo che la creazione della pipe di indice i vada a buon fine */
		{
			printf("Errore nella creazione della pipe di comunicazione tra padre e figlio di indice i = %d\n", i);
			exit(3);
		}
	}

	for (i = 0; i < N; i++)		/* Itero un ciclo per creare gli N processi filgi */
	{
		if((pid = fork()) < 0)	/* Controllo che la creazone dell'i-esimo processo figlio vada a buon fine */
		{
			printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
			exit(4);
		}
		if (pid == 0)
		{
			/* Processo figlio */
			for (j = 0; j < N; ++j)		/* Itero un ciclo per chiudere tutte le pipe sul lato di lettura */
			{
				close(piped[j][0]);
				if(i != j)		/* Nel caso i sia diverso da j allora chiudo anche il lato in scrittura */
				{
					close(piped[j][1]);
				}
			}

			if (pipe(p) < 0)		/* Controllo che la pipe di comunicazione tra nipote e figlio sia creata con successo */
			{
				printf("Errore nella creazione della pipe di comunicazione tra nipote e figlio di indice i = %d\n", i);
				exit(-1);
			}
			
			if ((pid = fork()) < 0)		/* Controllo che il processo nipote sia creato con successo */
			{
				printf("Errore nella creazione del processo nipote di indice i = %d\n", i);
				exit(-1);
			}
			if(pid == 0)
			{
				/* Processo nipote */
				close(piped[i][1]);	/* Chiudo la pipe rimasta aperta del processo figlio in lato scrittura */
				close(0);		/* Chiudo la pipe di comunicazione tra processo nipote e figlio in lato scrittura */
				if((open(argv[i + 1], O_RDONLY)) < 0)		/* Controllo che l'apertura dell'i-esimo parametro vada a buon fine */
				{
					printf("Errore nell'apertura del file %s in modalita' lettura\n", argv[i + 1]);
					exit(-1);
				}
				
				close(1);
				dup(p[1]);

				close(p[0]);
				close(p[1]);

				close(2);		/* Chiudo lo standard error e lo ridirigo su /dev/null */
				open("/dev/null", O_WRONLY);

				execlp("wc", "wc", "-l", (char*)0);	/* Eseguo il comando filtro wc sul file aperto in lettura */
				exit(-1);		/* Nel caso si arrivi a questo punto vuol dire che qualcosa e' fallito nella exec */
			}
			/* Codice figlio */
			close(p[1]);			/* Chiudo il lato non usato della pipe del processo nipote */

			j = 0;
			while(read(p[0], &(numero[j]), 1))		/* Itero un ciclo che legge dalla pipe p */
			{
				++j;
			}
			
			if(j != 0)			/* Nel caso j sia diverso da zero allora ha delle righe il file */
			{
				numero[j - 1] = '\0';	/* Aggiungo il terminatore all'array */
				printf("%s e' il numero trovato\n", numero);
				valore = atoi(numero);	/* Converto il numero in valore */
			}
			else
			{
				valore = 0;		/* Nel caso non ci siano righe allora valore = 0 */
			}

			write(piped[i][1], &valore, sizeof(valore));	/* Scrivo nella i-esima pipe tra padre e figlio il valore salvato */
			ritorno = -1;			/* Se il processo nipote e' terminato in modo anomalo allora deve terminare con -1 */
			
			if((pidFiglio = wait(&status)) < 0)
			{
				printf("Errore nella wait\n");
			}
			else if((status & 0xFF) != 0)
			{
				printf("Processo nipote con PID: %d e' terminato in modo anomalo\n", pidFiglio);
			}
			else
			{
				ritorno = (int)((status >> 8) & 0xFF);
				exit(ritorno);
			}
		}
	}

	/* Processo padre */
	
	for (i = 0; i < N; ++i)			/* Itero un ciclo che chiude tutte le pipe non usate */
	{
		close(piped[i][1]);
	}

	for (i = 0; i < N; ++i)			/* Recupero tutte le informazioni dalle pipe dei processi figli */
	{
		read(piped[i][0], &valore, sizeof(valore));
		somma += valore;
	}

	printf("La somma risultante di tutte le righe trovate e': %ld\n", somma);

	for (i = 0; i < N; ++i)			/* Ciclo che aspetta tutti i valori dai processi figli */
	{
		if((pidFiglio = wait(&status)) < 0)	/* Controllo che la wait abbia successo */
		{
			printf("Errore nella wait\n");
		}
		if((status & 0xFF) != 0)
		{
			printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
		}
		else
		{
			ritorno = (int)((status >> 8) & 0xFF);
			if(ritorno == 255)
			{
				printf("Processo figlio con PID: %d terminato con valore %d quindi significa che il figlio ha avuto problemi oppure il nipote non e' riuscito ad eseguire il wc oppure e' terminato in modo anomalo\n", pidFiglio, ritorno);
			}
			else
			{
				printf("Il figlio con PID: %d e' terminato con valore %d\n", pidFiglio, ritorno);
			}
		}
	}
	
	exit(0);
}
