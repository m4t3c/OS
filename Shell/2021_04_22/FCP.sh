#!/bin/sh
#Script principale che fa:
#1. Il controllo dei parametri
#2. Invocazione ricorsiva per ogni directory
#3. Stampa del numero totale di directory trovate
#4. Chiede all'utente un numero compreso tra 1 e X
#5. Stampa il nome della Xesima directory salvata

#Controllo che siano passati almeno 4 parametri
case $# in
0|1|2|3)
		echo "ERRORE: Hai passato $# parametri mentre ne sono richiesti almeno 4"
		echo "Usage is: $0 numint string dir1 dir2"
		exit 1;;
*)
		;;
esac

#Definisco le variabili per contenere il primo e il secondo parametro
W=
S=

#Controllo che il primo parametro sia un numero intero strettamente positivo
case $1 in
*[!0-9]*)
		echo "ERRORE: $1 non è un numero o non è strettamente positivo"
		exit 2;;
*)
		if test $1 -eq 0
		then
			echo "ERRORE: $1 è uguale a 0 mentre deve essere maggiore o uguale ad 1"
			exit 3
		fi;;
esac

case $2 in
*/*)
	echo "ERRORE: $2 non è una semplice stringa"
	exit 4;;
*)
	;;
esac

W=$1
S=$2
shift
shift

#Controllo che i restanti parametri siano nomi assoluti di directory traversabili
for Q
do
	case $Q in
	/*)
		if test ! -d $Q -o ! -x $Q
		then
			echo "ERRORE: $Q non directory o non traversabile"
			exit 5
		fi;;
	*)
		echo "ERRORE: $Q non è un nome assoluto"
		exit 6;;
esac

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo
> /tmp/nomiAssoluti

for G
do
	FCR.sh $G $S /tmp/nomiAssoluti
done

#Stampo il numero totale di directory trovate
ND=`wc -l > /tmp/nomiAssoluti`
echo "Numero di directory: $ND"

#Controllo che ND sia maggiore o uguale di W
if test $ND -ge $W
then
	#Chiedo all'utente di dirmi un numero compreso tra 1 e W
	echo -n "Letizia dimmi un numero compreso tra 1 e $W: "
	read X
	case $X in
	*[!0-9]*)
			echo "ERRORE: $X non è un numero o non è strettamente positivo"
			rm /tmp/nomiAssoluti
			exit 7;;
	*)
			if test $X -lt 1 -o $X -gt $W
			then
				echo "ERRORE: il parametro $X non è compreso tra 1 e $W"
				rm /tmp/nomiAssoluti
				exit 8
			else
				echo "L'$X-esima directory è: `head -$X /tmp/nomiAssoluti | head -1`"
			fi;;
	esac
fi

rm /tmp/nomiAssoluti
