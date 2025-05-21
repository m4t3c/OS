#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Definisco il tipo pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv)
{
	int pid;			/* Per la fork */
	int N;				/* Numero di caratteri passati come parametri */
	int fdr;			/* File descriptor per la open */
	int n;				/* Indice dei processi */
	int k;				/* Indice */
	long int count;			/* Per conteggio */
	char c;				/* Buffer del carattere letto */
	pipe_t *piped;			/* Array dinamico di pipe */
	int pidFiglio, status, ritorno;	/* Per wait */
	
	if(argc < 4)			/* Controllo che il numero di parametri passati sia corretto */
	{
		printf("Errore nel numero di parametri: ho bisogno di almeno 3 parametri (nome file, n caratteri) ma argc = %d\n", argc);
		exit(1);
	}
	
	N = argc - 2;
	
	for (n = 0; n < N; ++n)		/* Controllo che tutti i caratteri passati siano dei caratteri singoli */
	{
		if((strlen(argv[n + 2])) != 1)
		{
			printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[n + 2]);
			exit(2);
		}
	}

	piped = (pipe_t *)malloc(N * sizeof(pipe_t));	/* Alloco N pipe */
	if(piped == NULL)		/* Controllo che l'allocazione della pipe vada a buon fine */
	{
		printf("Errore nell'allocazione dell'array dinamico per le pipe\n");
		exit(3);
	}

	for (n = 0; n < N; ++n)		/* Creio N pipe figli-padre */
	{
		if(pipe(piped[n]) < 0)	/* Controllo che la creazione della pipe vada a buon fine */
		{
			printf("Errore nella creazione della pipe di indice %d\n", n);
			exit(4);
		}
	}

	for (n = 0; n < N; ++n)
	{
		if((pid = fork()) < 0)	/* Controllo che la creazione dell'n-esimo processo figlio vada a buon fine */
		{
			printf("Errore nella creazione del processo figlio di indice %d\n", n);
			exit(5);
		}
		if(pid == 0)
		{
			for (k = 0; k < N; ++k)	/* Itero un ciclo su k che mi servira' per chiudere in scrittura le pipe che non mi servono */
			{
				close(piped[k][0]);
				if(k != n)
				{
					close(piped[k][1]);
				}
			}

			if((fdr = open(argv[1], O_RDONLY)) < 0)
			{
				printf("Errore nell'apertura del file %s\n", argv[1]);
				exit(-1);
			}

			count = 0L;
			
			while((read(fdr, &c, 1)) > 0)
			{
				if (c == argv[n+2][0])
				{
					count++;
				}
			}

			/* Comunico al padre il numero di caratteri trovati */
			write(piped[n][1], &count, sizeof(count));

			exit(argv[n + 2][0]);
		}
	}
	
	/* Processo padre */
	for (n = 0; n < N; n++)		/* Itero un ciclo che chiude tutte le pipe in scrittura */	
	{
		close(piped[n][1]);
	}

	/* Leggo tutti i messaggi delle pipe */
	for (n = 0; n < N; ++n)
	{
		if((read(piped[n][0], &count, sizeof(count))) > 0)	/* Controllo che la lettura abbia successo */
		{
			printf("Trovate %ld occorrenze del carattere %c nel file %s\n", count, argv[n + 2][0], argv[1]);
		}
	}

	for (n = 0; n < N; ++n)		/* Ciclo che fa la wait su tutti gli N processi figli */
	{
		if((pidFiglio = wait(&status)) < 0)	/* Controllo che la wait vada a buon fine */
		{
			printf("Errore nella wait\n");
			exit(6);
		}
		if((status & 0xFF) != 0)
		{
			printf("Figlio con PID %d terminato in modo anomalo\n", pidFiglio);
		}
		else
		{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Il processo figlio con PID: %d ha ritornato %c (se 255 problemi!)\n", pidFiglio, ritorno);
		}
	}

	exit(0);
}
