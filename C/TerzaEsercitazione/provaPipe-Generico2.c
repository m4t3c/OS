/* Programma in C provaPipe-Generico1.c che accetta un singolo parametro che rappresenta un file con un numero generico di linee strettamente minore di 255 (compreso il terminatore di linea). Dopo aver letto una linea deve mandare al padre la lunghezza della stringa compreso il terminatore e poi deve mandare la stringa. Il padre usa la lunghezza della stringa per leggere il numero di caratteri e stamparli a mano a mano */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int pid, j, piped[2];		/* pid per fork, contatore j, piped per la pipe */
	char mess[255];			/* Char per contenere ciascuna linea letta */
	int fd;				/* Variabile per la open usata dal figlio */
	int i;				/* Indice per la lettura di un singolo carattere */
	int pidFiglio, status, ritorno;	/* Variabili usate per la wait */

	/* Controllo che sia passato un solo parametro */
	if(argc != 2)
	{
		printf("Errore nel numero di parametri: %s vuole un solo parametro ma argc = %d\n", argv[0], argc);
		exit(1);
	}

	/* Creo una pipe e controllo che la creazione abbia successo */
	if(pipe(piped) < 0)
	{
		printf("Errore creazione pipe\n");
		exit(2);
	}
	
	/* Creo il processo figlio */
	if((pid = fork()) < 0)
	{
		printf("Errore creazione del filgio\n");
		exit(3);
	}
	if(pid == 0)
	{
		/* Processo figlio */
		close(piped[0]);	/* Chiudo il lato di lettura della pipe */
		if((fd = open(argv[1], O_RDONLY)) < 0)	/* Controllo che il file passato come parametro esista e sia apribile in lettura */
		{
			printf("Errore nell'apertura del file %s\n", argv[1]);
			exit(-1);
		}

		/* Setto i contatori a 0 */
		i = 0;
		j = 0;

		while(read(fd, &(mess[i]), 1) != 0)	/* Leggo carattere per carattere dal file aperto */
		{
			if(mess[i] == '\n')
			{
				mess[i] = '\0';
				i++;

				write(piped[1], mess, i);		/* Stampo sulla pipe la linea */
				i = 0;
				j++;
			}
			else
			{
				i++;
			}
		}
		exit(j);
	}

	/* Processo padre */
	close(piped[1]);
	j = 0;
	i = 0;
	while((read(piped[0], &mess[i], 1)) != 0)
	{
		if(mess[i] == '\0')
		{
			printf("%d: %s\n", j, mess);
			i = 0;
			j++;
		}
		else
		{
			++i;
		}
	}

	if((pidFiglio = wait(&status)) < 0)
	{
		printf("Errore wait\n");
		exit(5);
	}
	if((status & 0xFF) != 0)
	{
		printf("Figlio con PID: %d terminato in modo anomalo\n", pidFiglio);
	}
	else
	{
		ritorno = ((status >> 8) & 0xFF);
		printf("Il figlio con PID: %d e' terminato %d (se 255 problemi)\n", pidFiglio, ritorno);
	}

	exit(0);
}
	
