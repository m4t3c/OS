#!/bin/sh
#Script che controlla che:
#1. Siano passati almeno 3 parametri
#2. Il primo parametro sia un numero intero Y strettamente positivo
#3. I restanti N devono essere nomi assoluti di directory

#Controllo che i parametri siano almeno 3
case $# in
0|1|2)	
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)	
	;;
esac

#Controllo che il primo parametro sia numerico e strettamente positivo
case $1 in
*[!0-9]*)	echo "ERRORE: non numerico o non strettamente positivo"
		exit 2;;
*)		if test $1 -eq 0
		then
			echo "ERRORE: parametro non diverso da 0"
			exit 3
		fi;;
esac
#Salvo il primo parametro nella variabile Y
Y=$1
shift

#Scorro i restanti n parametri per controllare che siano tutti nomi assoluti di directory
for i
do
	case $i in
	/*)	
		if test ! -d $i -o ! -x $i
		then
			echo "ERRORE non directory o non traversabile"
			exit 4
		fi;;
	*)	
		echo "ERRORE: parametro passato non in nome assoluto"
		exit 5;;
	esac
done

#Aggiorno la variabile PATH
PATH=$PATH:`pwd`
export PATH

#creo un file temporaneo dove mettere i percorsi assoluti
> /tmp/conta$$

#Con un for esploro le directory passate
for i
do	
	echo DEBUG-chiamo il file ricorsivo con $i
	FCR.sh $i $Y /tmp/conta$$
done

#Stampo il numero di file creati
echo Numero di file totali creati: `wc -l /tmp/conta$$`
#stampo i nomi assoluti dei file creati
for i in `cat /tmp/conta$$`
do
	echo Creato il file $i
	#controllo che abbia 5 righe almeno
	case $i in
	*NO*)	echo "Il file originale non aveva almeno 5 righe quindi è vuoto";;
	*)	echo "il contenuto del file $i è il seguente:"
		echo ===
		cat < $i
		echo ===
		;;
	esac
done

#elimino il file temporaneo
rm /tmp/conta$$
