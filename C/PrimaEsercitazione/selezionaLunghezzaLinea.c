#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char **argv)
{
	int F, n;												//Variabili per salvare i parametri
	char buffer[255];											//Creo un buffer per contenere una linea
	int j = 0;												//Variabile indice del buffer
	int trovate = 0;											//Counter delle linee trovate
	if(argc != 3)												//Controllo che siano passati esattamente 2 parametrri
	{
		printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
		exit(1);
	}
	
	if((F = open(argv[1], O_RDONLY)) < 0)									//Controllo che il primo parametro sia un file
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file\n", argv[1]);
		exit(2);
	}

	if((n = atoi(argv[2])) < 0)										//Contorllo che il secondo parametro sia strettamente positivo
	{
		printf("Errore nel passaggio dei parametri: %s non e' un numero strettamente positivo\n", argv[2]);
		exit(3);
	}
	
	while(read(F, &buffer[j], 1))										//Avvio un ciclo che legge carattere per carattere il file F
	{
		if(buffer[j] == '\n')										//Sono arrivato ad una papabile riga
		{
			if(n == j + 1)										//Controllo che la riga abbia n caratteri
			{
				buffer[j + 1] = '\0';
				printf("%s\n", buffer);
				trovate++;
			}
			j = 0;
		}
		else
		{
			++j;
		}
	}
	
	if(trovate == 0)
	{
		printf("Non esiste nessuna riga lunga %d caratteri\n", n);
	}
	
	exit(0);
}
