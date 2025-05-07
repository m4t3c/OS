#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int n;															//Variabile per salvare il numero di righe chieste dall'utente
	int i = 1;														//Counter delle righe
	char c;															//Buffer dei caratteri
	int f;															//Variabile per salvare il file nella open

	if(argc > 3)														//Controllo che siano passati massimo 2 parametri
	{
		printf("Errore nel numero dei parametri: ho bisogno di massimo due parametri ma argc = %d\n", argc);
		exit(1);
	}
	
	if(argc >= 2)
	{
		if(argv[1][0] != '-')
		{
			printf("Errore nel passaggio dei parametri: non hai passato un'opzione del comando head\n");		//Controllo che il parametro sia un'opzione
			exit(2);
		}	
	
		if((n = atoi(&argv[1][1])) < 0)											//Controllo che il numero passato sia strettamente maggiore di 0
		{	
			printf("Errore nel passaggio dei parametri: %d non e' strettamente positivo\n", n);
			exit(3);
		}
	}
	else
	{
		n = 10;
	}	

	if(argc == 3)
	{
		if((f = open(argv[2], O_RDONLY)) < 0)										//Controllo che il secondo parametro sia un file
		{
			printf("Errore nel passaggio dei parametri: %s non e' un file\n", argv[2]);
			exit(4);
		}
		while(read(f, &c, 1) == 1)											//Avvio un ciclo di lettura dal file
		{
			write(1, &c, 1);
			if(c == '\n')
			{
				++i;
				if(i > n)
				{
					break;
				}
			}
		}
	}
	else
	{
		while(read(0, &c, 1) == 1)											//Avvio un ciclo che legge da standard input e scrive su standard output
		{
			write(1, &c, 1);
			if(c == '\n')												//Ogni volta che si incontra il carattere a capo aumento il counter
			{
				++i;
				if(i > n)											//Se ho superato il numero di righe si interrompe il ciclo e il programma termina
				{
					break;
				}
			}		
		}
	}	
	
	exit(0);
}
