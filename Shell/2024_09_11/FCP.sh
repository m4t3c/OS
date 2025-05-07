#!/bin/sh
#File principale che fa:
#1. Il controllo dei parametri
#2. L'invocazione del file ricorsivo FCR.sh
#3. L'invocazione della parte in C

#Controllo che siano passati almeno 3 parametri
case $# in
0|1|2)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)
	;;
esac

#Creo tre variabili in cui salvo i nomi delle directory, l'ultimo parametro e un counter
list=
X=
num=0

#Scorro tutti i parametri passati
for G
do
	num=`expr $num + 1`
	#Controllo di non essere nell'ultimo parametro
	if test $num -ne $#
	then
		#Controllo che il parametro sia un nome assoluto di una directory traversabile
		case $G in
		/*)
			if test ! -d $G -o ! -x $G
			then
				echo "ERRORE: $G non directory o non traversabile"
				exit 3;;
			fi
			list="$list $G";;
		*)
			echo "ERRORE: $G non nome assoluto"
			exit 4;;
		esac
	else
		#Controllo che il parametro sia strettamente positivo
		case $G in
		*[!0-9]*)
				echo "ERRORE: $G non e' un numero o non e' strettamente positivo"
				exit 5;;
		*)
				if test $G -eq 0
				then
					echo "ERRORE: il parametro passato non e' diverso da 0"
				fi
				X=$G;;
		esac
	fi
done

#Setto la variabile PATH e la esporto
PATH=`pwd`:$PATH
export PATH

#Creo i file temporanei per directory e file
> /tmp/nomid
> /tmp/nomif

#Definisco la variabile livello per tenere conto del livello in cui siamo
livello=0

#Invoco il file ricorsivo per ogni Gerarchia
for G in $list
do
	FCR.sh $G $X $livello /tmp/nomid /tmp/nomif
done

rm /tmp/nomi*

