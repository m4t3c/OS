#!/bin/sh
#Script principale per:
#1. controllare che sia passato un parametro
#2. controllare che il parametro passato sia un nome assoluto di una directory traversabile
#3. Invocare il file ricorsivo per la prima fase
#4. Stampare il numero di livelli trovati
#5. Chiedere all'utente un numero dispari
#6. Invocare il file ricorsivo per la seconda fase

#Controllo i parametri
case $# in
1)
	case $1 in
	/*)
		if test ! -d $1 -o ! -x $1
		then
			echo "ERRORE: $1 non directory o non traversabile"
			exit 1
		fi;;
	*)	
		echo "ERRORE: $1 non in nome assoluto"
		exit 2;;
	esac;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne è richiesto 1 - Usage [directory]"
	exit 3;;
esac

#creo il file temporaneo che conterrà il numero massimo del livello trovato
> /tmp/livello$$

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo le variabili fase e livello
fase=1
livello=-1

#Invoco il file ricorsivo
FCR.sh $1 $livello $fase /tmp/livello$$

#Salviamo il numero di livelli trovati in una variabile NL e rimuoviamo il file temporaneo
NL=`cat /tmp/livello$$`
rm /tmp/livello$$

#Stampiamo il numero di livelli trovati
echo "Numero di livelli trovati nell'esplorazione della gerarchia $1 è: $NL"

#Chiediamo all'utente un numero dispari compreso tra 1 e NL
echo -n "Dimmi un numero dispari compreso tra 1 e $NL: " > /dev/tty
read X

#Controlli sulla variabile X
case $X in
*[!0-9]*)
		echo "ERRORE: il valore $X non è un numero oppure non è positivo"
		exit 4;;
*)
		#Controllo che il valore di X sia compreso tra 1 e NL
		if test $X -lt 1 -o $X -gt $NL
		then
			echo "ERRORE: il valore $X non è compreso tra 1 e $NL"
			exit 5
		else
			#Controllo che il valore X sia dispari
			if test `expr $X % 2` -eq 0
			then
				echo "ERRORE: il valore $X non è dispari"
				exit 6
			fi
		fi;;
esac

#Setto la variabile fase a 2
fase=2

#Invoco il file ricorsivo con la seconda fase
FCR.sh $1 $livello $fase $X

