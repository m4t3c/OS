#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int pid;				/* per fork */
	int pidFiglio, status, ritorno;		/* per wait padre */

	if(argc != 2)		/* Controllo che sia passato un solo parametro */
	{
		printf("Errore nel numero di parametri: ho bisogno di 1 parametro ma argc = %d\n", argc);
		exit(1);
	}

	if((pid = fork()) < 0)		/* Caso in cui la fork non vada a buon fine */
	{
		printf("Errore nella fork\n");
		exit(2);
	}
	if(pid == 0)
	{
		/* Processo figlio */
		execlp("ls", "ls", "-l", argv[1], (char *)0);
		printf("Errore nell'execl\n");
		exit(-1);
	}

	/* Processo padre */
	if((pidFiglio = wait(&status)) < 0)	/* Controllo che la wait venga eseguita correttamente */
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
		printf("Figlio con PID: %d terminato con valore %d (se 255 problemi!)\n", pidFiglio, ritorno);
	}

	exit(0);
}

