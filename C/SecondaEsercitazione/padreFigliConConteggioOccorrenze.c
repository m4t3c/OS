#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	char Cx;		/* Ultimo carattere */
	int N;			/* Numero di file passati */
	int n;			/* Indice dei figli */
	int pid;		/* pid per fork */
	int f;			/* Variabile per aprire il file */
	int c;			/* Variabile che conta le occorrenze del carattere Cx */
	char buffer;		/* Buffer dei caratteri letti */
	int pidFiglio, status, ritorno;  /* per wait e valore di ritorno figli */

	if(argc < 4)		/* Controllo che siano passati almeno tre parametri */
	{
		printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri ma argc = %d\n", argc);
		exit(1);
	}

	N = argc - 2;		/* Salvo il numero di file passati */
	if(strlen(argv[argc - 1]) != 1)		/* Controllo che l'ultimo parametro sia un singolo carattere */
	{
		printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[argc - 1]);
		exit(2);
	}
	Cx = argv[argc - 1];

	for(n = 0; n < N; ++n)	/* Inizializzo un for per creare N processi filgi */
	{
		if((pid = fork()) < 0)
		{
			printf("Errore nella fork\n");
			exit(3);
		}
		if(pid == 0)
		{
			if((f = open(argv[n + 1])) < 0)
			{
				printf("Errore: %s non e' un file o non e' apribile\n", argv[n + 1]);
				exit(-1);
			}
			c = 0;
			while(read(f, &buffer, 1))
			{
				if(buffer == Cx)
				{
					++c;
				}
			}
			exit(c);
		}
	}
	
	for(n = 0; n < N; ++n)		/* Inizializzo un ciclo per la wait */
	{
		pidFiglio = wait(&status);
		if(pidFiglio < 0)
		{
			printf("Errore nel processo figlio\n");
			exit(4);
		}
		if((status & 0xFF) != 0)
		{
			printf("Figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
			exit(5);
		}
		else
		{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Nel file %s sono state trovate %d occorrenze del carattere %c\n", argv[n + 1], ritorno, Cx);
		}
	}

	exit(0);
}
