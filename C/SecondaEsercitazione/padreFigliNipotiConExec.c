#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PERM 0644

int main(int argc, char **argv)
{
	int N;					/* Numero di parametri passati */
	int n;					/* Indice del ciclo */
	int pid;				/* Per fork */
	char *FOut;				/* Array dinamico di char per il nome del file da creare da parte dei figli */
	int fdw;				/* Per la creat */
	int pidFiglio, status, ritorno;		/* Per wait */

	if (argc < 4)
	{
		printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomi di file) ma argc = %d\n", argc);
		exit(1);
	}
	
	N = argc - 1;

	for(n = 0; n < N; n++)
	{
		if((pid = fork()) < 0)
		{
			printf("Errore creazione figlio numero %d\n", n);
			exit(2);
		}
		if(pid == 0)
		{
			/* Processo figlio */
			FOut = (char*)malloc(strlen(argv[n + 1]) + 6);		/* La memoria allocata deve contenere la stringa del nome del file associato al processo + i caratteri .sort+ il terminatore */	
			if(FOut == NULL)
			{
				printf("Errore nell'allocazione della memoria\n");
				exit(-1);
			}

			strcpy(FOut, argv[n+1]);
			strcat(FOut, ".sort");
			if((fdw = creat(FOut, PERM)) < 0)
			{
				printf("Impossibile creare il file %s.sort\n", argv[n+1]);
				exit(-1);
			}
			close(fdw);

			if((pid = fork()) < 0)		/* Creo il processo nipote */
			{
				printf("Errore nella fork del nipote\n");
				exit(-1);
			}
			if(pid == 0)
			{
				/* Processo nipote */

				close(0);	/* Chiudo lo standard input e lo reindirizzo sul file passato come parametro */
				if(open(argv[n + 1], O_RDONLY) < 0)
				{
					printf("Errore: il file %s non esiste o non e' leggibile\n", argv[n + 1]);
					exit(-1);
				}

				close(1);	/* Chiudo lo standard output e lo reindirizzo sul file creato argv[n + 1].sort */
				if(open(FOut, O_WRONLY) < 0)
				{
					printf("Errore: il file %s non si riesce ad aprire in scrittura\n", FOut);
					exit(-1);
				}

				execlp("sort", "sort", (char *)0);

				perror("Problemi di scrittura nel nipote\n");
				exit(-1);
			}
		}
	}
		
	/* Processo padre */
	for(n = 0; n < N; ++n)
	{
		if((pidFiglio = wait(&status)) < 0)
		{
			printf("Errore nella wait\n");
			exit(3);
		}
		if((status & 0xFF) != 0)
		{
			printf("Figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
			exit(4);
		}
		else
		{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Il figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
		}
	}
		
	exit(0);
}


