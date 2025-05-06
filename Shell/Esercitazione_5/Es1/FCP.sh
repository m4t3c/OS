#!/bin/sh
#script che controlla se:
#1. Siano passati almeno 3 parametri
#2. N (>= 2) siano nomi asoluti di directory
#3. Le directory esistano e siano traversabili
#4. L'ultimo parametro sia un numero strettamente positivo

#definisco le variabili list e X che mi serviranno rispettivamente per salvare i nomi delle directory e il numero intero X
#inoltre definisco una variabile count che mi serve per tenere conto a quele parametro sono arrivato
list=
X=
count=0

#controllo il numero dei parametri
case $# in
0|1|2)	echo "ERRORE: sono stati passati $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)	echo DEBUG-numero di parametri corretto;;
esac

#con un ciclo for scorro tutti i parametri passati
for i
do
	#incremento count di 1
	count=`expr $count + 1`
	#controllo se i non è l'ultimo parametro
	if test $count -ne $#
	then
		#controllo che la directory sia in nome assoluto
		case $i in
		/*) 	#controllo che sia una directory e che sia traversabile
				if test ! -d $i -o ! -x $i
				then
					echo "ERRORE: non directory o non traversabile"
					exit 2
				fi
		    	#se soddisfa i controlli la salvo in list
		    	list="$list $i";;
		*)	echo "ERRORE: parametro non in nome assoluto"
			exit 3;;
		esac
	else
		#controllo che l'ultimo parametro sia strettamente positivo
		if test $i -lt 1
		then
			echo "ERRORE: l'ultimo parametro vale $i mentre deve essere > di 0"
			exit 4
		fi
		X=$i
	fi
done

#Aggiorno la variabile PATH
PATH=$PATH:`pwd`
export PATH 

#Invoco il file ricorsivo FCR.sh scorrendo tutti gli elementi di list
for i in $list
do
	echo Esploro la gerarchia $i
	FCR.sh $i $X
done


