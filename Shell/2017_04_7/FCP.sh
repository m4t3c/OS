#!/bin/sh
#Script principale che fa:
#1. Il controllo dei parametri
#2. L'invocazione dello script ricorsivo FCR.sh
#3. La stampa su standard output del numero totale di file sorted creati globalmente
#4. La stampa su standard output del nome assoluto di ogni file e in seguito la prima e l'ultima riga di quel file

#Controllo che siano passati almeno 3 parametri
case $# in
0|1|2)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	echo "Usage is: $0 file dirass1 dirass2 ..."
	exit 1;;
*)
	;;
esac

#Controllo che il primo parametro sia un nome relativo semplice di un file
case $1 in
*/*)
	echo "ERRORE: $1 non è un nome relativo semplice"
	exit 2;;
*)
	;;
esac

#Salvo il primo parametro in una variabile F e poi eseguo uno shift per avere come primo parametro il primo nome assoluto della directory

F=$1
shift

#Controllo che i restanti parametri siano nomi assoluti di directory traversabili
for G
do
	case $G in
	/*)
		if test ! -d $G -o ! -x $G
		then
			echo "ERRORE: $G non directory o non traversabile"
			exit 3
		fi;;
	*)
		echo "ERRORE: $G non è un nome assoluto"
		exit 4;;
	esac
done

#Aggiorno la variabile PATH e la esporto
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo che contenga i nomi assoluti dei file ordinati
> /tmp/nomiAssoluti

for i
do
	FCR.sh $i $F /tmp/nomiAssoluti
done

#Salvo in una variabile NF il numero di file che sono stati trovati e ordinati
NF=`wc -l /tmp/nomiAssoluti`

#Stampo il numero di file trovati
echo "Il numero di file sorted creati è: $NF"

#Scorro tutti i file creati e per ognuno stampo il suo nome assoluto e visualizzo la prima e l'ultima riga
for F in `cat /tmp/nomiAssoluti`
do
	echo "Nome assoluto del file: $F"
	echo "==="
	echo "Prima riga del file: `head -1 $F`"
	echo "==="
	echo "Ultima riga del file: `tail -1 $F`"
	echo "==="
done

