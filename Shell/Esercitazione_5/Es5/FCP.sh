#!/bin/sh
#Script che controlla che
#1. Siano passati almeno 4 parametri
#2. Il primo parametro sia un numero intero strettamente positivo
#3. Il secondo parametro sia una stringa S
#4. I restanti parametri siano nomi assoluti di directory che identificano Q gerarchie

#Definisco le variabili che mi servono per salvare il numero W e la stringa S
W=
S=

#Controllo che i parametri siano almeno 4
case $# in
0|1|2|3)	
		echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 4"
		exit 1;;
*)		
		;;
esac

#Controllo che il primo parametro sia strettamente positivo
case $1 in
*[!0-9]*)	
		echo "ERRORE: il primo parametro non è un numero strettamente positivo"
		exit 2;;
*)		
		if test $1 -eq 0
		then
			echo "ERRORE: il primo parametro non è diverso da 0"
			exit 3;;
		fi
esac

#Salvo il primo parametro in W e poi eseguo uno shift per rendere il secondo parametro il primo
W=$1
shift

#Controllo che il nuovo primo parametro sia una stringa semplice
case $1 in
*/*)		
		echo "ERRORE: la stringa $1 potrebbe rappresentare un nome assoluto o relativo"
		exit 4;;
*)		
		;;
esac

#Salvo la stringa nella variabile S ed eseguo uno shift
S=$1
shift

#Scorro i restanti parametri e controllo che siano nomi assoluti di directory
for i
do
	case $i in
	/*)	
		if test ! -d $i -o ! -x $i
		then
			echo "ERRORE: $i non directory o non traversabile"
			exit 5
		fi;;
	*)	
		echo "ERRORE: $i non è un nome assoluto"
		exit 6;;
	esac
done

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo un file temporaneo
> /tmp/nomioassoluti$$

#Scorro le directory invocando il file ricorsivo FCR.sh
for Q
do
	FCR.sh $Q $S /tmp/nomiassoluti$$
done

#Scrivo su standard output il numero di directory trovate che soddisfano i requisiti

NL=`wc -l < /tmp/nomiassoluti$$`
echo "Numero di directory trovate: $NL"

#Nel caso ci siano abbastanza directory da essere maggiori o uguali a W chiedo all'utente di dirmi un numero tra 1 e W e stampo il nome assoluto della iesima directory
if test $NL -ge $W
then
	echo -n "Matteo dimmi un numero compreso tra 1 e $W "
	read Answer
	case Answer in
	#Nel caso la risposta non sia valida elimino il file temporaneo e termino il programma
	*[!0-9]*)	
			rm /tmp/nomiassoluti$$
			exit 7;;
	*)		#controllo che il numero sia strettamente positivo e minore di W
			if test $Answer -eq 0 -o $Answer -gt $W
			then
				rm /tmp/nomiassoluti$$
				exit 8
			fi;;
	esac
	#Se i controlli sono stati superati allora stampo a video il nome assoluto della directory
	name=`head -$Answer < /tmp/nomiassoluti$$ | tail -1`
	echo "La $Answer-esima directory è: $name"
else	
	echo "$NL è minore di W"
fi

#cancello il file temporaneo
rm /tmp/nomiassoluti$$
