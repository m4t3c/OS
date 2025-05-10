#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int mia_random(int n) 
{ 
	int casuale; 
	casuale = rand() % n; 
	return casuale; 
}

int main()
{
	int pid;							//Variabile per salvare il valore del PID del figlio
	int pidfiglio, status, ritorno;					//Variabili da usare per il controllo della wait
	int rand;							//Variabile per salvare il numero random	

	printf("PID del processo padre: %d\n", getpid());
	srand(time(NULL));

	if((pid = fork()) < 0)
	{
		printf("Errore nella fork\n");
		exit(1);
	}
	if(pid == 0)
	{
		/* processo figlio */
		printf("PID del processo: %d\nPPID del processo padre: %d\n", getpid(), getppid());
		rand = mia_random(100);
		return(rand);
	}

	/* processo padre */
	if((pidfiglio = wait(&status)) < 0)
	{
		printf("Errore: si e' verificato un errore nella wait\n");
		exit(2);
	}

	if((status & 0xFF) != 0)					//Il bit alto deve essere 0
	{
		printf("Figlio con pid %d terminato in modo anomalo\n", pidfiglio);
	}
	else
	{
		ritorno = (int)((status >> 8) & 0xFF);
		printf("Il figlio con PID = %d ha ritornato %d (Se 255 o 0 problemi!)\n", pidfiglio, ritorno);
	}

	exit(0);
}
