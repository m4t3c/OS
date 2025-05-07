#!/bin/sh
#Script principale che fa:
#1. Il controllo dei parametri
#2. Crea il file temporaneo per salvare le directory che soddisfano le caratteristiche
#3. Invoca lo script ricorsivo FCR.sh
#4. Stampa il numero totale di directory trovate
#5. Chiede all'utente Elena se vuol visualizzare il contenuto della directory D

#Controllo che siano passati almeno 3 parametri
case $# in
0|1|2)
	echo "ERRORE: Hai passato $# parametri mentre ne sono richiesti almeno 3"
	echo "Usage is: $0 carattere dirass1 dirass2 ..."
	exit 1;;
*)
	;;
esac

#Controllo che il primo parametro sia un singolo carattere
case $1 in
?)
	;;
*)	
	echo "ERRORE: $1 non e' un singolo carattere"
	exit 2;;
esac

#Salvo il carattere nella variabile C ed eseguo uno shift per poter svolgere in maniera piu' agevole i controlli sui restanti parametri
C=$1
shift

#Controllo che i restanti paramentri siano nomi assoluti di directory traversabili
for D
do
	case $D in
	/*)
		if test ! -d $D -o ! -x $D
		then
			echo "ERRORE: $D non directory o non traversabile"
			exit 3
		fi;;
	*)
		echo "ERRORE: $D non e' un nome assoluto"
		exit 4;;
	esac
done

#Aggiorno la variabile PATH e la esporto per compatibilita'
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo per salvare i nomi assoluti delle directory
> /tmp/nomiAssoluti



rm /tmp/nomiAssoluti
