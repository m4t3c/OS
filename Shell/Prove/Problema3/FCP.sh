#!/bin/sh
#Script che esegue le seguenti operazioni:
#1. Controlla che siano passati 2 parametri
#2. Controlla che il primo parametro sia un nome assoluto di una directory traversabile
#3. Controlla che il secondo parametro sia un nome relativo semplice
#4. Invoca ricorsivamente FCR.sh
#5. Stampa su standard output i nomi assoluti delle directory che sono state trovate nella gerarchia che soddisfano i requisiti

#Controllo il numero di parametri
case $# in
2)	
	;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti 2 - Usage is $0 dirass DR"
	exit 1;;
esac

#Controllo che il primo parametro sia un nome assoluto di una directory traversabile
case $1 in
/*)
	if test ! -d $1 -o ! -x $1
	then
		echo "ERRORE: $1 non directory o non traversabile"
		exit 2
	fi;;
*)
	echo "ERRORE: $1 non è un nome assoluto"
	exit 3;;
esac

#Controllo che il secondo parametro sia un nome relativo semplice
case $2 in
*/*)
	echo "ERRORE: $2 non è un nome relativo semplice"
	exit 4;;
*)
	;;
esac

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo che conterrà i nomi assoluti delle directory
> /tmp/tmp$$

#Invoco lo script ricorsivo
FCR.sh $1 $2 /tmp/tmp$$

#Stampo il numero di directory trovate
ND=`wc -l < /tmp/tmp$$`
echo "Numero di directory trovate $ND"

#Rimuovo il file temporaneo
rm /tmp/tmp$$
