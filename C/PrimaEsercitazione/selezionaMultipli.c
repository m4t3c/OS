#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int n;														//Variabile per salvare il secondo parametro
	int f;														//Variabile per aprire il file passato come primo parametro
	int nread;													//Variabile per contare il numero di caratteri letti

	if(argc != 3)													//Controllo che siano passati esattamente 2 parametri
	{
		printf("Errore nel numero dei parametri: ho bisogno di due parametri ma argc = %d\b", argc);
		exit(1);
	}

	if((f = open(argv[1], O_RDONLY)) < 0)										//Controllo che il primo parametro sia un file apribile in lettura
	{
		printf("Errore nel passaggio dei parametri: %s non e' un file o non e' apribile in lettura", argv[1]);
		exit(2);
	}

	if((n = atoi(argv[2])) < 0)											//Controllo che il secondo parametro sia un numero strettamente positivo
	{
		printf("Errore nel passaggio dei parametri: %d non e' un numero strettamente positivo", n);
		exit(3);
	}

	char *buff = malloc(n * sizeof(char));										//Creo un array allocato dinamicamente di dimensione n
	for(int i = 1; (nread = read(f, buff, n)) == n; ++i)								//Con un ciclo for scorro tutti gli n-esimi caratteri del file passato
	{
		printf("Il carattere multiplo %d all'interno del file %s e' %c\n", i, argv[1], buff[n - 1]);
	}

	exit(0);
}

