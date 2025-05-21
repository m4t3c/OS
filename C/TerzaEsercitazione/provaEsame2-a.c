#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct	{
			char c;
			long int n;
		}TipoS;

int main(int argc, char **argv)
{
	int pid;				/* Per fork */
	int N;					/* Numero dei caratteri */
	int fdr;				/* File descriptor della open */
	int n;					/* Indice per la creazione dei processi figli */
	int c;					/* Buffer per il carattere letto */
	int p[2];				/* Pipe singola */
	TipoS msg;				/* Struct per comunicare tra figlio e padre */
	int pidFiglio, ritorno, status;

	if(argc < 4)				/* Controllo che il numero di parametri sia corretto */
	{
		printf("Errore nel numero dei parametri: %s ha bisogno di almeno 3 parametri ma argc = %d\n", argv[0], argc);
		exit(1);
	}

	N = argc - 2;

	for (n = 0; n < N; ++n)			/* Itero un ciclo che controlla gli N caratteri */
	{
		if(strlen(argv[n + 2]) != 1)	/* Controllo che l'n-esimo parametro sia un singolo carattere */
		{
			printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[n + 2]);
			exit(2);
		}
	}
	
	if(pipe(p) < 0)				/* Controllo che la pipe sia creata correttamente */
	{
		printf("Errore nella creazione della pipe\n");
		exit(3);
	}

	for(n = 0; n < N; ++n)			/* Itero un ciclo che crea gli N processi figli */
	{
		if((pid = fork()) < 0)
		{
			printf("Errore nella fork\n");
			exit(4);
		}
		if(pid == 0)
		{
			/* Processo figlio */
			close(p[0]);

			if((fdr = open(argv[1], O_RDONLY)) < 0)		/* Controllo che l'apertura del file vada a buon fine */
			{
				printf("%s non e' un file o non e' apribile in lettura\n", argv[1]);
				exit(-1);
			}
			
			/* Inizializzo la struct */
			msg.c = argv[n+2][0];		/* Carattere associato */
			msg.n = 0L;			/* Contatore occorrenze */

			while(read(fdr, &c, 1))		/* Itero un ciclo finche' si legge da file */
			{
				if(c == msg.c)		/* Controllo se il carattere letto e' uguale al carattere da cercare */
				{
					msg.n++;
				}
			}
			
			write(p[1], &msg, sizeof(msg));
			exit(msg.c);
		}
	}

	/* Processo padre */
	close(p[1]);

	while((read(p[0], &msg, sizeof(msg))) > 0)			/* Itero un ciclo finche' si legge dalla pipe */
	{
		printf("Trovate %ld occorrenze del carattere %c nel file %s\n", msg.n, msg.c, argv[1]);
	}

	if((pidFiglio = wait(&status)) < 0)				/* Controllo che la wait vada a buon fine */
	{
		printf("Errore wait\n");
		exit(5);
	}
	if((status & 0xFF) < 0)
	{
		printf("Processo figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
	}
	else
	{
		ritorno = (int)((status >> 8) & 0xFF);
		printf("Il processo figlio con PID: %d e' terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
	}
	
	exit(0);
}	

