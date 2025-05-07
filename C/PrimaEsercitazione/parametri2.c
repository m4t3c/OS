#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv)
{
	int N;						//Variabile per salvare il secondo parametro
	char C;						//Variabile per salvare il terzo parametro
	
	if(argc != 4)					//Controllo che il numero di parametri sia giusto
	{
		printf("Errore nel numero dei parametri: ho bisogno di esattamente 3 parametri ma argc = %d\n", argc);
		exit(1);
	}

	if(open(argv[1], O_RDONLY) < 0)			//Controllo che il primo parametro sia un file
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file\n", argv[1]);
		exit(2);
	}

	if((N = atoi(argv[2])) <= 0)			//Controllo che il secondo parametro sia maggiore di zero 
	{
		printf("Errore nel passaggio dei parametri: %d non e' maggiore di 0\n", N);
		exit(3);
	}

	if(strlen(argv[3]) != 1)			//Controllo che il terzo parametro sia un solo carattere
	{
		printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[3]);
		exit(4);
	}

	C = argv[3][0];

	printf("All'eseguibile %s sono stati passati %d parametri\n", argv[0], argc - 1);
	printf("Il primo parametro corrisponde al file %s\n", argv[1]);
	printf("Il secondo parametro corrisponde al numero %d\n", N);
	printf("Il terzo parametro corrisponde al carattere %c\n", C);
	
	exit(0);
}
