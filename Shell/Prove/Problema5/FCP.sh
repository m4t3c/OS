#!/bin/sh
#Script che fa:
#1. Il controllo che siano passati 3 parametri
#2. Il controllo che il primo parametro sia una directory traversabile passata in nome assoluto
#3. Il controllo che i restanti 2 parametri E1 e E2 siano due stringhe semplici che rappresentano estensioni
#4. L'invocazione del file ricorsivo FCR.sh

#Definisco la variabile per salvare la gerarchia G
G=

#Controllo il numero di parametri
case $# in
3)
	;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti 3 - Usage is [directory] E1 E2"
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
	echo "ERRORE: $1 non in nome assoluto"
	exit 3;;
esac

#Salvo il primo parametro in G ed eseguo shift
G=$1
shift

#Controllo che i restanti parametri siano stringhe semplici
for i
do
	case $i in
	*/*)
		echo "ERRORE: $i non è una stringa semplice"
		exit 4;;
	*)
		;;
	esac
done

#Controllo che le due stringhe non siano uguali
if test $1 = $2
then
	echo "ERRORE: le due stringhe $1 e $2 sono uguali"
	exit 5
fi

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Invoco il file ricorsivo
FCR.sh $G $*
