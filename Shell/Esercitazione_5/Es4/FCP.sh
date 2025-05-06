#!/bin/sh
#Script che controlla che:
#1) ci siano almeno 3 parametri passati
#2) il primo parametro sia un carattere
#3) gli altri N parametri siano nomi assoluti di directory

#definisco le variabili C e list
C=						#variabile C per inserire il carattere del primo parametro

#controllo che siano passati almeno 3 parametri
case $# in
0|1|2)	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)	;;
esac

#controllo che il primo parametro sia un solo carattere
case $1 in
?)	;;
*)	echo "ERRORE: il primo parametro non e' costituito da un solo carattere"
	exit 2;;
esac

#inserisco il primo parametro nella variabile C e poi eseguo uno shift
C=$1
shift

#eseguo un for che scorra tutti gli N parametri rimanenti
for i
do	case
	/*)	if test ! -d $i -o ! -x $i
		then	echo "ERRORE: $i non e' una directory o non e' traversabile"
			exit 3;;
		fi
	*)	echo "ERRORE: $i non e' un nome assoluto"
		exit 4;;
	esac
done

#aggiorno la variabile di ambiente PATH

PATH=`pwd`:$PATH
export PATH

#creo il file temporaneo dove salvare i percorsi assoluti delle directory
> /tmp/nomiAssoluti

#invochiamo N volte il file ricorsivo FCR.sh

for i
do
	FCR.sh $i $C /tmp/nomiAssoluti
done

#nel file /tmp/nomiAssoluti ci sono i nomi assoluti delle directory che hanno soddisfatto le specifiche
#scriviamo su standard output il numero di directory valide
echo "Numero totale di directory trovate: `wc -l < /tmp/nomiAssoluti`"
echo	#stampiamo una riga vuota per leggibilità

#eseguo un for che scorre tutti i nomi assoluti del file temporaneo e chiedo ad Elena se vuole visualizzare il contenuto della directory
for i in `cat /tmp/nomiAssoluti`
do
	echo "Elena vuoi visualizzare il contenuto della directory? (si/no)"
	read answer
	case
	si) echo -----
		ls -la $i
	    echo -----;;
	no) ;;
	esac
done

rm /tmp/nomiAssoluti

