/* Programma in C append.c che se esiste il file passato come parametro, lo apre in scrittura, altrimenti lo crea; quindi, scrive su tale file tutto quello che viene letto dallo standard input */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define PERM 0644 /* in ottale per diritti UNIX */

int main(int argc, char **argv)
{
    int outfile, nread;  /* usati per open/creat e valore ritorno read */
    char buffer[BUFSIZ]; /* usato per i caratteri */

    if (argc != 2) /* controllo sul numero di parametri: deve essere esattamente 1! */
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        printf("Ci vuole 1 argomento: nome-file\n");
        exit(1);
    }

    /* Uso la primitiva nella sua forma estesa in maniera tale che se il file è già stato creato lo apro dalla fine in scrittura, altrimenti lo creo con i permessi definiti sopra */
    if ((outfile = open(argv[1], O_WRONLY | O_APPEND | O_CREAT, PERM)) < 0)
    {
        printf("Errore nella creazione/apertura del file %s\n", argv[1]);
        exit(2);
    }
    

    /* leggiamo dallo standard input */
    while ((nread = read(0, buffer, BUFSIZ)) > 0)
    {
        if (write(outfile, buffer, nread) < nread) /* scriviamo sul file */
        {
            exit(3); /* errore */
        }
    } /* fine del file di input */

    exit(0);
}