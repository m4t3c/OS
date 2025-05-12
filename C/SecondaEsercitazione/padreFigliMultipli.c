#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int N;		/* Numero dei figli */
	int pid;	/* pid per fork */
	int n;		/* indice per i figli */
	int pidFiglio, status, ritorno;		/* per wait e valore di ritorno filgi */
	
	if(argc != 2)	/* Controllo che sia passato un solo parametro */
	{
		printf("Errore nel numero dei parametri: ho bisogno di un parametro ma argc = %d\n", argc);
		exit(1);
	}

	if((N = atoi(argv[1])) <= 0 || N >= 255)	/* Controllo che il parametro sia positivo e che sia minore di 255 */
	{
		printf("Errore nel passaggio dei parametri: %s non e' strettamente positivo o non e' minore di 255\n", argv[1]);
		exit(2);
	}

	printf("PID del processo padre: %d\nNumero passato: %d\n", getpid(), N);	/* Stampo il pid del padre e il numero passato */
	
	for(n = 0; n < N; ++n)		/* Creo un ciclo for per creare tutti i processi figli */
	{
		if((pid = fork()) < 0)
		{
			printf("Errore nella fork\n");
			exit(3);
		}	
		if (pid == 0)
		{
			printf("PID del processo figlio numero %d: %d\n", n + 1, getpid());
			exit(n);
		}
	}
	
	for(n = 0; n < N; ++n)	/* Creo un ciclo for per la wait */
	{
		pidFiglio = wait(&status);
		if(pidFiglio < 0)
		{
			printf("Errore: si e' verificato un errore nella wait\n");
			exit(4);
		}

		if((status & 0xFF) != 0)
		{
			printf("Figlio con PID %d terminato in modo anomalo\n", pidFiglio);
			exit(5);
		}
		else
		{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Il figlio con pid %d ha ritornato il valore %d (se 255 o 0 problemi)\n", pidFiglio, ritorno);
		}
	}
	exit (0);
}
