#!/bin/sh
#Script che fa:
#1. Il controllo che sia passato un solo parametro
#2. Il controllo che il parametro sia un nome assoluto di una directory traversabile
#3. Invoca ricorsivamente il file FCR.sh
#4. Chiede all'utente un numero pari tra 1 e il numero di livelli totali della gerarchia
#5. Richiama il file ricorsivo FCR.sh

#Controllo del numero di parametri
case $# in
1)
	;;
*)
	echo "ERRORE: hai passato $# paramentri mentre ne è richiesto solo 1"
	exit 1;;
esac

#Controllo che il parametro sia un nome assoluto di una directory traversabile
case $1 in
/*)
	if test ! -d $1 -o ! -x $1
	then
		echo "ERRORE: $1 non directory o non traversabile"
		exit 2
	fi;;
*)
	echo "ERRORE: $1 non è in nome assoluto"
	exit 3;;
esac

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Definisco le variabili che mi serviranno
conta=0						#variabile per contare i livelli
fase=A						#variabile che determina in che fase dello script ci troviamo

echo $conta > /tmp/tmpContaLivelli		#Scrivo il valore di conta nel file temporaneo

#Invoco FCR.sh
FCR.sh $1 $conta $fase

read tot < /tmp/tmpContaLivelli
echo "Numero totale di livelli: $tot"

#Passiamo alle fase B
fase=B

#Chiedo all'utente un numero compreso tra 1 e tot che debba essere pari
echo -n "Dimmi un numero pari tra 1 e $tot: "
read livello
case $livello in
*[!0-9]*)
		echo "ERRORE: $livello non numero o non positivo"
		rm /tmp/tmpContaLivelli
		exit 4;;
*)
		#Controllo che sia compreso tra 1 e tot
		if test $livello -ge 1 -a $livello -le $tot
		then
			#controllo che il numero sia pari
			if test `expr $livello % 2` -ne 0
			then
				echo "ERRORE: $livello non è pari"
				rm /tmp/tmpContaLivelli
				exit 5;;
			fi
		else
			echo "ERRORE: $livello non è compreso tra 1 e $tot"
			rm /tmp/tmpContaLivelli
			exit 6;;
		fi
esac

echo $conta > /tmp/tmpContaLivelli

#Torno ad invocare il file ricorsivo
FCR.sh $1 $conta $fase $livello

rm /tmp/tmpContaLivelli
