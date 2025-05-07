CC=gcc
CFLAGS=-Wall -std=c11

all: parametri1 parametri2 provaBUFSIZ contaOccorrenze mycat1 selezionaMultipli sostituisciCar myhead1 myhead2 myhead3

parametri1: parametri1.c
	$(CC) $(CFLAGS)	-o parametri1 parametri1.c

parametri2: parametri2.c
	$(CC) $(CFLAGS) -o parametri2 parametri2.c

provaBUFSIZ: provaBUFSIZ.c
	$(CC) $(CFLAGS) -o provaBUFSIZ provaBUFSIZ.c

contaOccorrenze: contaOccorrenze.c
	$(CC) $(CFLAGS) -o contaOccorrenze contaOccorrenze.c

mycat1: mycat1.c
	$(CC) $(CFLAGS) -o mycat1 mycat1.c

selezionaMultipli: selezionaMultipli.c
	$(CC) $(CFLAGS) -o selezionaMultipli selezionaMultipli.c

sostituisciCar: sostituisciCar.c
	$(CC) $(CFLAGS) -o sostituisciCar sostituisciCar.c

myhead1: myhead1.c
	$(CC) $(CFLAGS) -o myhead1  myhead1.c

myhead2: myhead2.c
	$(CC) $(CFLAGS) -o myhead2 myhead2.c

myhead3: myhead3.c
	$(CC) $(CFLAGS) -o myhead3 myhead3.c
clean:
	rm -rf *.o
