#!/bin/sh
#Script che esegue i seguenti comandi:
#1. controlla che siano passati almeno 3 parametri
#2. controlla che il primo parametro passato sia un file in nome relativo semplice
#3. controlla che i restanti N parametri siano nomi assoluti di directory eseguibili
#4. genera il file temporaneo /tmp/conta$$
#5. aggiorna la variabile PATH
#6. invoca ricorsivamente FCR.sh
#7. Riporta su standard output il nome assoluto del file e riporta la prima e l'ultima linea del file

#Definisco la variabile F che conterrà il primo parametro
F=

#Controllo i parametri
case $# in
0|1|2)
	echo "ERRORE: hai passato $# parametri mentre devono essere almeno 3"
	exit 1;;
*)
	echo DEBUG-numero di parametri corretto;;
esac

#Controllo che il primo parametro sia un nome relativo semplice di un file
case $1 in
*/*)
	echo "ERRORE: $1 non è un nome relativo semplice"
	exit 2;;
*)
	;;
esac

#eseguo uno shift per rendere il primo parametro la prima directory
F=$1
shift

#Controllo che i restanti parametri siano tutti nomi assoluti di directory traversabili
for i
do
	case $i in
	/*)
		if test ! -d $i -o ! -x $i
		then
			echo "ERRORE: $i non directory o non traversabile"
			exit 3
		fi;;
	*)
		echo "ERRORE: $i non è un nome assoluto"
		exit 4;;
	esac
done

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo il file temporaneo che conta il numero di file e che contiene i vari nomi assoluti
> /tmp/conta$$

#Invoco ricorsivamente FCR.sh per tutte le directory passate
for G
do
	FCR.sh $G $F /tmp/conta$$
done

#Creo la variabile che contiene il numero di file creati
NF=`wc -l < /tmp/conta$$`

#Stampo il numero di file creati
echo "Numero di file creati: $NF"

#Per ogni file creato stampo nome assoluto, prima riga e ultima riga
for file in `cat /tmp/conta$$`
do
	echo "Nome assoluto del file $file"
	#Controllo che il file sia leggibile per poter stampare le righe
	if test -r $file
	then
		echo ===
		head -1 $file
		echo ===
		tail -1 $file
	        echo ===
	fi
done

#Elimino il file temporaneo
rm /tmp/conta$$
