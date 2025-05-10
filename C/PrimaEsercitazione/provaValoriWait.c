/* Programma in C status1.c che non ha bisogno di parametri e mostra l'uso della primitiva fork per la creazione di un processo figlio e della primitiva wait per l'attesa del figlio da parte del padre, con recupero del valore tornato dal figlio */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 

int main ()
{
     int pid; 				/* pid per fork */
     int pidFiglio, status, ritorno;    /* per wait padre */
     int valore;

	if ((pid = fork()) < 0)
	{ 	/* fork fallita */
		printf("Errore in fork\n");
		exit(1);
	}

	if (pid == 0)
	{ 	/* figlio */
		printf("Esecuzione del figlio con PID = %d\n", getpid());
		scanf("%d", &valore);
		if(valore > 255 || valore < 0)
		{
			printf("Il valore sara' troncato\n");
			exit(-1);
		}
		else
		{
			printf("Il valore non sara' troncato\n");
			exit(valore);					/* si torna un valore derivante dall'esecuzione del processo figlio */
		}
	}

	/* padre */
	printf("Generato figlio con PID = %d\n", pid);

	/* il padre aspetta il figlio in questo caso interessandosi del valore della exit del figlio */
	if ((pidFiglio=wait(&status)) < 0)
	{
		printf("Errore in wait\n");
		exit(2);
	}

	/* il padre analizza il valore di status per capire se il figlio e' terminato in modo anomalo cioe' involontario */
	if ((status & 0xFF) != 0)
    		printf("Figlio terminato in modo involontario (cioe' anomalo)\n");
    	else	/* oppure se e' terminato in modo normale e in tal caso recupera il valore ritornato */
    	{
    		/* selezione del byte "alto" */
    		ritorno = status >> 8;
    		ritorno &= 0xFF;
		/* o anche direttamente:
		ritorno=(int)((status >> 8) & 0xFF);  */
		/* stampa del valore 'pulito' ritornato dal figlio */
    		printf("Il figlio %d ha ritornato il valore intero %d\n", pidFiglio, ritorno);
    	}

	exit(0);
}
