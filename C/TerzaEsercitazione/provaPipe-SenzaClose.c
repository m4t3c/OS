/* Programma in C provaPipe.c che ha bisogno di un singolo parametro che deve essere un file con uno specifico formato: numero non noto di linee (ma strettamente minore di 255) e ogni linea composta da 4 caratteri (oltre il terminatore di linea '\n'); il processo padre genera un processo figlio che legge via via le linee dal file; per ogni linea letta, la trasforma in stringa e la invia al padre che la riporta su standard output */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define MSGSIZE 5 

int main (int argc, char **argv)
{   
   int pid, j, piped[2]; 		/* pid per fork, j per contatore, piped per pipe */
   char mess[MSGSIZE];			/* array usato dal figlio per inviare la stringa al padre e array usato dal padre per ricevere la stringa inviata dal figlio */
   /* N.B. dato che ogni processo (padre e figlio) avra' la propria AREA DATI il contenuto di questo array sara' PRIVATO di ogni processo e quindi NON serve creare due array distinti per il padre e per il figlio! */
   /* La stessa cosa vale anche per tutte le altre variabili chiaramente! */
   int fd;				/* variabile usata dal figlio per open file */
   int pidFiglio, status, ritorno;      /* per wait padre */

	/* controllo sul numero di parametri: esattamente uno */
	if(argc != 2)
	{
		printf("Errore nel numero dei parametri: ho bisogno di un parametro (nome di file) ma argc = %d\n", argc);
		exit(1);
	}

	/* si crea una pipe: si DEVE sempre controllare che la creazione abbia successo!  */
	if((pipe(piped)) < 0)
	{
		printf("Errore nella creazione della pipe\n");
		exit(2);
	}
	/* si crea un figlio (controllando sempre!) */
	if((pid = fork()) < 0)
	{
		printf("Errore nella creazione del processo figlio\n");
		exit(3);
	}
	if (pid == 0)  
	{   
		/* figlio */
		if((fd = open(argv[1], O_RDONLY)) < 0)		/* Controllo che l'apertura del file vada a buon fine */
		{
			printf("Errore: %s non e' un file o non e' apribile in lettura\n", argv[1]);
			exit(-1);
		}
		j = 0;		/* Inizializzo il contatore j a 0 */

		while(read(fd, mess, MSGSIZE))		/* Inizializzo un ciclo che va avanti finche' leggo 4 caratteri da una riga del file passato come parametro */
		{
			mess[MSGSIZE - 1] = '\0';
			write(piped[1], mess, MSGSIZE);	/* Mando al processo padre il messaggio che verra' consumato */
			j++;
		}

		exit(j);
	}
	/* padre */
	
	j=0; /* il padre inizializza la sua variabile j per verificare quanti messaggi gli ha mandato il figlio */

	while(read(piped[0], mess, MSGSIZE))	/* Inizializzo un ciclo che va avanti finche' ci sono messaggi da leggere dalla pipe */
	{
		printf("%d: %s\n", j, mess);
		++j;
	}

	if((pidFiglio = wait(&status)) < 0)	/* Controllo che la wait abbia successo */
	{
		printf("Errore nella wait\n");
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
		printf("Il figlio con PID: %d e' terminato con %d (se 255 problemi!)\n", pidFiglio, ritorno);
	}

	exit(0);
}
