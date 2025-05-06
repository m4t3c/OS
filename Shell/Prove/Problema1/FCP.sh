#!/bin/sh
#Script che esegue:
#1. il controllo che vengano passati esattamente 1 parametro
#2. il controllo che il parametro passato sia una directory, che sia traversabile e che sia in nome assoluto
#3. stampa il numero di file contati e recupera il contenuto di ogni file se leggibile

#Controllo dei parametri
case $# in
1)
	;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne è richiesto 1"
	exit 1;;
esac

#Controllo che il parametro sia una directory in nome assoluto
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

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo per salvare i nomi assoluti dei file
> /tmp/nomiAssoluti$$

#Invoco ricorsivamente FCR.sh
FCR.sh $1 /tmp/nomiAssoluti$$

#Vado a stampare il numero di elementi trovati
NE=`wc -l < /tmp/nomiAssoluti$$`
echo "Numero di elementi trovati: $NE"

#Con un ciclo for che esplora tutti gli elementi del file temporaneo stampo il contenuto del file trovato se leggibile
for i in `cat /tmp/nomiAssoluti$$`
do
	echo "Nome assoluto del file: $i"
	#Controllo che il file sia leggibile
	if test -r $i
	then
		echo "Il suo contenuto è"
		echo ===
		cat $i
		echo ===
	fi
done

#Elimino il file temporaneo
rm /tmp/nomiAssoluti$$
