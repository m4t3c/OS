#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int F;												//Variabile per salvare il file passato come primo parametro quando lo si apre in lettura
	char Cx;											//Variabile per salvare il carattere salvato come secondo parametro
	char buff;											//Creo la variabile buff che servira' per la read
	
	if(argc != 3)											//Controllo che siano passati 3 parametri
	{
		printf("Errore nel numero dei parametri: ho bisogno di 2 parametri ma argc = %d\n", argc);
		exit(1);
	}

	if((F = open(argv[1], O_RDWR) < 0))								//Controllo che il primo parametro sia un file apribile in lettura e scrittura
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file\n", argv[1]);
		exit(2);
	}

	if((strlen(argv[2])) != 1)									//Controllo che il secondo parametro sia un singolo carattere
	{
		printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[2]);
		exit(3);
	}

	Cx = argv[2][0];										//Salvo il parametro nella variabile Cx
	
		
	while(read(F, &buff, 1) > 0)								//Creo un ciclo che continua finche' ci sono caratteri da leggere
	{
		printf("DEBUG-Leggo il carattere %c dal file", buff);
		if(buff == Cx)										//Controllo se il carattere letto e' quello da sostituire
		{
			lseek(F, -1L, SEEK_CUR);							//Torno indietro di uno nella lettura per modificare il carattere
			write(F, " ", SEEK_CUR);							//Modifico buff con il carattere vuoto
		}
	}
	
	exit(0);
}
