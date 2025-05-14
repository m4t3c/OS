#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int fd;					/* Per open */
	int pid;				/* Per fork */
	int pidFiglio, status, ritorno;		/* Per wait */

	if(argc != 2)
	{
		printf("Errore nel numero dei parametri: ho bisogno di 1 parametro (nome del file da stampare) ma argc = %d\n", argc);
		exit(1);
	}
	
	if((pid = fork()) < 0)			/* Controllo che la fork vada a buon fine */
	{
		printf("Errore nella fork\n");
		exit(2);
	}
	if(pid == 0)
	{
		/* Processo figlio */
		close(0);			/* Chiudo lo standard input per reindirizzarlo su standard input */
		if((fd = open(argv[1], O_RDONLY) < 0))
		{
			printf("Errore in apertura del file %s\n", argv[1]);
			exit(-1);
		}
		
		execl("./mycat", "./mycat", (char *)0);

		printf("Errore in exec\n");
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
		printf("Figlio con PID: %d ritornato in modo anomalo\n", pidFiglio);
	}
	else
	{
		ritorno = (int)((status >> 8) & 0xFF);
		printf("Il figlio con PID: %d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
	}

	exit(0);
}

