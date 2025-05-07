#!/bin/sh
#Script principale che fa:
#1. Il contorllo dei parametri
#2. Invoca il file ricorsivo per ogni directory
#3. Controlla il numero totale di file trovati
#4. Nel caso sia 0 stampa un errore
#5. Nel caso contrario si stampa il numero, poi si crea una directory nella directory corrente con un link HW ad ogni file trovato nella 3 posizione all' interno delle directory delle varie gerarchie

#Controllo che siano passati almeno 3 parametri
case $# in
0|1|2)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	echo "Usage is $0 stringa dirass1 dirass2 ..." 
	exit 1;;
*)
	;;
esac

#Definisco la variabile X dove andro' a salvare il primo parametro
X=

#Controllo che il primo paramentro sia un nome relativo semplice, affinche' sia tale non deve contenere il carattere "/"
case $1 in
*/*)
	echo "ERRORE: $1 non e' un nome relativo semplice in quanto contiene /"
	exit 2;;
*)
	;;
esac

#Salvo il primo parametro nella variabile e poi eseguo uno shift
X=$1
shift

#Controllo che i restanti Q parametri siano nomi assoluti di directory traversabili
for Q
do
	case $Q in
	/*)
		if test ! -d $Q -o ! -x $Q
		then
			echo "ERRORE: $Q non directory o non traversabile"
			exit 3
		fi;;
	*)
		echo "ERRORE: $Q non e' un nome assoluto"
		exit 4;;
	esac
done

#Creo il file temporaneo dove mettere i nomi assoluti dei file trovati
> /tmp/nomiAssoluti

#Aggiorno la variabile PATH e la esporto per compatibilita'
PATH=`pwd`:$PATH
export PATH

#Invoco ricorsivamente lo script ricorsivo per ogni directory
for G
do
	FCR.sh $G /tmp/nomiAssoluti
done

#Salvo in una variabile NRO il numero di file trovati in terza posizione
NRO=`wc -l < /tmp/nomiAssoluti`

#Se il NRO = 0 stampo una stringa di errore
if test $NRO -eq 0
then
	echo "ERRORE: il numero di file trovati in terza posizione e' uguale a 0"
	rm /tmp/nomiAssoluti
	exit 5;;
fi

#Stmampo il numero di file trovati
echo "Abbiamo trovato $NRO file"

#Creo la nuova directory con nome relativo semplice X
mkdir $X

#Scorro il file temporaneo e creo un nuovo link hardware per ogni file trovato in terza posizione
for $F in `cat /tmp/nomiAssoluti`
do
	ln $F $X
done

#Cancello il file temporaneo
rm /tmp/nomiAssoluti
