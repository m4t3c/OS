#!/bin/sh
#Script che fa:
#1. Il controllo che siano passati almeno 4 parametri
#2. Il contorllo che i primi due parametri siano due semplici stringhe
#3. Il controllo che i restanti Q parametri siano nomi assoluti di directory traversabili
#4. La creazione delle variabili e dei file temporanei necessari
#5. L'invocazione dello script ricorsivo FCR.sh per ogni directory
#6. In caso il numero di file con la prima stringa sia minore o uguale di quelli con la seconda stampa il nome assoluto dell'iesimo file con S1 e di quello con S2

#Controllo parametri
case $# in
0|1|2|3)
		echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 4-Usage is S1 S2 [dir1] [dir2] ... [dirn]"
		exit 1;;
*)
		;;
esac

#Definisco le variabili per salvare le stringhe S1 e S2
S1=
S2=

#Controllo che i primi due parametri siano semplici stringhe
case $1 in
*/*)
	echo "ERRORE: $1 non è una semplice stringa"
	exit 2;;
*)
	;;
esac
S1=$1

case $2 in
*/*)
	echo "ERRORE $2 non è una semplice stringa"
	exit 3;;
*)
	;;
esac

S2=$2
shift; shift

#Faccio il controllo sulle directory
for G
do
	case $G in
	/*)
		if test ! -d $G -o ! -x $G
		then
			echo "ERRORE: $G non directory o non traversabile"
			exit 4
		fi;;
	*)
		echo "ERRORE: $G non è un nome assoluto di directory"
		exit 5;;
	esac
done

#Creo i file temporanei che contengano i file che terminano con S1 e con S2
> /tmp/nomiAssolutiS1
> /tmp/nomiAssolutiS2

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Invoco ricorsivamente FCR.sh per ogni Q gerarchia
for Q
do
	FCR.sh $Q $S1 $S2 /tmp/nomiAssolutiS1 /tmp/nomiAssolutiS2
done

#Stampo il numero totale di file trovati
NF1=`wc -l /tmp/nomiAssolutiS1`
NF2=`wc -l /tmp/nomiAssolutiS2`
echo "Numero di file trovati con $S1: $NF1"
echo "Numero di file trovati con $S2: $NF2"

#Creo le variabili TOT1 e TOT2 per salvare il numero di file con S1 e S2 rispettivamente
TOT1=`wc -l < /tmp/nomiAssolutiS1`
TOT2=`wc -l < /tmp/nomiAssolutiS2`

#Controllo che TOT1 sia minore o uguale di TOT2
if test $TOT1 -le $TOT2
then
	#In caso chiedo all'utente di dirmi un numero compreso tra 
	echo -n "Letizia dimmi un numero compreso tra 1 e $TOT1: "
	read X
	#Controllo che X sia compreso tra 1 e TOT1
	if test $X -ge 1 -a $X -le $TOT1
	then
		#Stampo il nome assoluto dell'Xesimo file con S1 e dell'Xesimo file con S2
		echo "File numero $X che termina con $S1: `head -$X /tmp/nomiAssolutiS1 | head -1`"
		echo "File numero $X che termina con $S2: `head -$X /tmp/nomiAssolutiS2 | head -1`"
	fi
fi

rm /tmp/nomiAssoluti*
