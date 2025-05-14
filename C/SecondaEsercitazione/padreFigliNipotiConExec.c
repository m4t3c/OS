#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PERM 0644

int main(int argc, char **argv)
{
	int N;					/* Numero di parametri passati */
	int n;					/* Indice del ciclo */
	int pid;				/* Per fork */
	char *FOut;				/* Array dinamico di char per il nome del file da creare da parte dei figli */
	int fdw;				/* Per la creat */
	int pidFiglio, status, ritorno;		/* Per wait */

	if (argc < 4)
	{
		print("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomi di file) ma argc = %d\n", argc);
		exit(1);
	}
	
	N = argc - 1;

	for(n = 0; n < N; n++)
	{
		if((pid = fork()) < 0)
		{
			printf("Errore creazione figlio numero %d\n", n);
			exit(2);
		}
		if(pid == 0)
		{
			FOut=(char *)malloc(strlen(argv[n + 1]) + 6);		/* Bisogna allocare memoria per il nome del file passato + il carattere '.' + i caratteri della parola sort + il terminatore */
			if(FOut == NULL)
			{
				printf("Errore nella malloc\n");
				exit(-1);
			}
			
			strcpy(FOut, argv[n + 1]);	/* Copio il nome del file associato al processo figlio */
			strcat(FOut, ".sort");		/* Concateno la stringa specificata dal testo */
			
			if((fdw = creat(FOut, PERM)) < 0)
			{
				
