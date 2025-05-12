#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

int mia_random(int n)
{
	int casuale;
	casuale = rand() % n;
	casuale++;
	return casuale;
}

int main(int argc, char **argv)
{
	int N;		/* numero di figli */
	int pid;	/* pid per fork */
	int n;		/* indice per i figli */
	int rand;	/* variabile per il numero random generato */
	int pidFiglio, status, ritorno;         /* per wait e valore di ritorno filgi */

	if(argc != 2)	/* Controllo che venga passato esattamente un parametro */
	{
		printf("Errore nel numero dei parametri: ho bisogno di un parametro ma argc = %d\n", argc);
		exit(1);
	}

	if((N = atoi(argv[1])) <= 0 || N >= 155)		/* Controllo che il parametro sia un numero positivo minore di 155 */
	{
		printf("Errore nel passaggio dei parametri: %s non e' un numero o non e' compreso tra 0 e 154 (estremi inclusi)\n", argv[1]);
		exit(2);
	}

	printf("Al processo padre con pid: %d, e' stato passato come parametro il numero %d\n", getpid(), N);
	srand(time(NULL));

	for(n = 0; n < N; ++n)		/* Creo un ciclo per inizializzare ogni processo figlio */
	{
		if((pid = fork()) < 0)
		{
			printf("Errore nella fork\n");
			exit(3);
		}
		if(pid == 0)
		{
			printf("Il processo figlio con PID = %d ha numero d'ordine: %d\n", getpid(), n);
			rand = mia_random(100 + n);
			exit(rand);
		}
	}

	for(n = 0; n < N; ++n)		/* Creo ciclo per la wait */
	{
		pidFiglio = wait(&status);
		if(pidFiglio < 0)
		{
			printf("Errore nella wait\n");
			exit(4);
		}
		if((status & 0xFF) != 0)
		{
			printf("Processo figlio con PID = %d terminato in modo anomalo\n", pidFiglio);
			exit(5);
		}
		else
		{
			ritorno = (int)((status) >> 8 & 0xFF);
			printf("Il processo figlio con PID = %d ha ritornato il seguente valore: %d\n", pidFiglio, ritorno);
		}
	}
	exit(0);
}
