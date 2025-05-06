#!/bin/sh
#Script che fa:
#1. Il controllo che siano passati 4 parametri
#2. Che il primo parametro sia il nome assoluto di una directory traversabile G
#3. Che il secondo e il terzo parametro siano numeri interi strettamente positivi N e H
#4. Che il quarto parametro sia un singolo carattere Cx
#5. Aggiorna la variabile PATH
#6. Invoca ricorsivamente FCR.sh
#7. Stampa il numero di directory trovate

#Creo le variabili che mi servono per memorizzare i parametri
G=
N=
H=
Cx=

#Controllo dei parametri
case $# in
4)
	;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti 4"
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

G=$1
shift

#Controllo che il secondo parametro sia un numnero intero strettamente positivo
case $1 in
*[!0-9]*)
		echo "ERRORE: $1 non è un numero"
		exit 4;;
*)
		if test $1 -eq 0
		then
			echo "ERRORE: $1 non diverso da 0"
			exit 5;;
		fi
esac

N=$1
shift

#Controllo che il terzo parametro sia un numero intero strettamente positivo
case $1 in
*[!0-9]*)
		echo "ERRORE: $1 non è un numero"
		exit 4;;
*)
		if test $1 -eq 0
		then
			echo "ERRORE: $1 non diverso da 0"
			exit 5;;
		fi
esac

H=$1
shift

#Controllo che il quarto parametro sia un carattere singolo
case $1 in
?)
	;;
*)
	echo "ERRORE: $1 non è un singlo carattere"
	exit 6;;
esac

Cx=$1

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Invoco lo script ricorsivo
./FCR.sh $G $N $H $Cx
