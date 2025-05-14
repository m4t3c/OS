#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int pid;				/* Per fork */
	int pidFiglio, status, ritorno;		/* Per wait padre */

	if(argc != 3)				/* Controllo che siano passati esattmente 2 parametri */
	{
		printf("Errore nel numero dei parametri: ho bisogno di 2 parametri (stringa da cercare e nome del file in cui cercare) ma argc = %d\n", argc);
		exit(1);
	}

	/* Genero il processo figlio con la fork */
	if((pid = fork()) < 0)		/* Controllo se la fork ha dei problemi */
	{
		printf("Errore nella fork\n");
		exit(2);
	}
	if(pid == 0)
	{
		/* Processo figlio */
		close(1);		/* Chiudo lo standard output per poter ridirezionare su /dev/null */
		open("/dev/null", O_WRONLY);
		
		close(2);		/* Chiudo lo standard error per poter ridirezionare su /dev/null */
		open("/dev/null", O_WRONLY);

		execlp("grep", "grep", argv[1], argv[2], (char *)0);
		
		/* Se l'exec fallisce non si vede nulla perche' e' reidirezionato tutto su /dev/null */

		exit(-1);
	}

	/* Processo padre */
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
		printf("Il processo figlio con PID: %d ha ritornoato %d (se 255 problemi!)\n", pidFiglio, ritorno);
		if(ritorno == 0)	/* Verifico il successo della grep */
		{
			printf("Quindi il filgio %d ha trovato la stringa %s nel file %s\n", pidFiglio, argv[1], argv[2]);
		}
		else
		{
			printf("Quindi il figlio %d NON ha trovato la stringa %s nel file %s\n", pidFiglio, argv[1], argv[2]);
		}
	}

	exit(0);
}
