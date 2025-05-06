#!/bin/sh
#Script che controlla che:
#1. siano passati 1 o 2 parametri
#2. il nome della directory sia assoluto
#3. la directory esista e sia traversabile
#4. il nome del file sia relativo semplice
#5. setta la variabile d'ambiente PATH e la esporta
#6. invoca il file comandi ricorsivo
#7. esegue le eventuali azioni finali

d=	#variabile per salvare o il primo parametro oppure la directory corrente
f=	#variabile per salvare il parametro che rappresenta il nome relativo semplice del file

#Controllo del numero di parametri
case $# in
0)
	echo "ERRORE POCHI PARAMETRI - Usage is: $0 [directory] file"
	exit 1;;
1)
	d=`pwd`
	f=$1;;
2)
	d=$1
	f=$2;;
*)
	echo "ERRORE: hai passato $# parametri mentre ne sono consentiti massimo 2"
	exit 2;;
esac

#Controllo che il nome della directory sia assoluto, che esista e che sia traversabile
case $d in
/*)
	if test ! -d $d -o ! -x $d
	then
		echo "ERRORE: $d non directory o non traversabile"
		exit 3
	fi
	;;
*)
	echo "ERRORE: $d non nome assoluto"
	exit 4;;
esac

#Controllo che il nome del file sia relativo semplice
case $f in
*/*)
	echo "ERRORE: $f non in nome relativo semplice"
	exit 5;;
*)
	;;
esac

#Setto la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Invoco il file ricorsivo
Cercafile.sh $d $f

echo "Finito tutto"
