#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv)
{
	int f, nr;												//Variabili per aprire il file passato come primo parametro e per contare i caratteri letti
	long int tot = 0;											//Variabile per salvare il numero totale di corrispondenze trovate
	char Cx;												//Variabile per salvare il secondo parametro
	char buff[1];												//Buffer per salvare il carattere letto
	
	if(argc != 3)												//Controllo che siano stati passati esattamente 2 parametri
	{
		printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
		exit(1);
	}

	if((f = open(argv[1], O_RDONLY)) < 0)									//Controllo che il primo parametro sia un file aprendolo in lettura
	{
		printf("Errore nel passaggio dei parametri: %s non file o non leggibile\n", argv[1]);
		exit(2);

	}
	
	if((strlen(argv[2])) != 1)										//Controllo che il secondo parametro sia un singolo carattere
	{
		printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[2]);
		exit(3);
	}

	Cx = argv[2][0];

	while((nr = read(f, buff, 1)) == 1)
	{
		if(buff[0] == Cx)
		{
			tot++;
		}
	}
	
	printf("Il carattere %c e' stato trovato %ld volte\n", Cx, tot);
	exit(0);
}
