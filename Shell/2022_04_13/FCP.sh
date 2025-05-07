#!/bin/sh
#Script principale che fa:
#1. Il controllo sui parametri
#2. Invoca ricorsivamente lo script ricorsivo FCR.sh per ogni directory
#3. La stampa di tutti i file trovati per ogni directory esplorata
#4. Controlla che i file nelle gerarchie siano uguali e nel caso stampa che le directory G1 e Gn sono uguali

#Controllo che siano passati almeno 3 parametri
case $# in
0|1|2)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	echo "Usage is: $0 numintpos dirass 1 dirass2 ..."
	exit 1;;
*)
	;;
esac

#Definisco la variabile per salvare il primo parametro X
X=

#Controllo che il primo parametro sia un numero strettamente positivo
case $1 in
*[!0-9]*)
		echo "ERRORE: $1 non numero o non strettamente positivo"
		exit 2;;
*)
		if test $1 -eq 0
		then
			echo "ERRORE: $1 e' uguale a 0 mentre dovrebbe essere strettamente positivo"
			exit 3
		fi;;
esac

#Salvo il primo parametro in X ed eseguo uno shift per avere come parametri passati solo i nomi assoluti delle directory
X=$1
shift

#Controllo che i restanti Q parametri siano nomi assoluti di directory traversabili
for $Q
do
	case $Q in
	/*)
		if test ! -d $Q -o ! -x $Q
		then
			echo "ERRORE: $Q non directory o non traversabile"
			exit 4
		fi;;
	*)
		echo "ERRORE: $Q non nome assoluto"
		exit 5;;
	esac
done

#Creo il counter n che incrementa ad ogni iterazione del for per avere un file temporaneo diverso ad ogni fase Q in cui salvare i file che rispettano le specifiche
n=0

for G
do
	#Incremento la variabile n
	n=`expr $n + 1`
	> /tmp/nomiAssoluti-$n
	FCR.sh $G $X /tmp/nomiAssoluti-$n
	#Stampo il numero di linee nel file temporaneo per avere il numero di file trovati
	echo "Numero di file trovati: `wc -l < /tmp/nomiAssoluti-$n`"
done

n=1
shift

#Scorro tutte le directory
for G
do
	#Incremento la variabile n saltando la prima directory
	n=`expr $n + 1`
	#Confronto i file salvati della prima directory con i file delle altre
	for F in `cat /tmp/nomiAssoluti-1`
	do
		#Scorro le altre directory
		for F1 in `cat /tmp/nomiAssoluti-1`
		do
			#confronto i due file con diff
			if diff $F $F1 > /dev/null 2>&1
			then
				echo "I file $1 e $G sono uguali"
			fi
		done
	done
done
