#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int N;									//Variabile di tipo int che salva il numero di parametri
	
	if(argc < 2)								//Controllo che sia passato almeno un parametro
	{
		printf("Errore nel numero di parametri: ho bisogno di almeno 1 parametro ma argc = %d/n", argc);
		exit(1);
	}

	N = argc;								//Salvo il numero di parametri passati nella variabile N
	printf("A %s sono stati passati %d parametri\n", argv[0], N - 1);	//Stampo a video il nome dell'eseguibile e il numero di parametri passati
	
	for(int n = 0; n < N - 1; n++)
	{
		printf("Elemento numero %d: %s\n", n + 1, argv[n + 1]);		//Stampo tutti i parametri passati

	}
	
	exit(0);
}
