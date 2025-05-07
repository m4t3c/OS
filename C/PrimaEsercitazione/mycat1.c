#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char **argv)
{	int nread;
	char buffer[BUFSIZ];
	int fd = 0;		/* Usato per la open, rimarra' 0 se non viene passato nessun parametro */
	
	for(int i = 1; i < argc; ++i)
	{
		/* Abbiamo un parametro che si presume essere un nome di file esistente e leggibile */
	  	/* Tentiamone l'apertura con una open in lettura */
	  	if ((fd = open(argv[i], O_RDONLY)) < 0)
	  	
		{
	  		printf("Errore in apertura file %s dato che fd = %d\n", argv[i], fd);
			exit(2);
	  	}

		printf("===Scrivo il contenuto del file: %s===\n", argv[i]);

	  	while ((nread = read(fd, buffer, BUFSIZ)) > 0 )
	  	{	
	  		write(1, buffer, nread);
	  	}
	}
	exit(0);
}
