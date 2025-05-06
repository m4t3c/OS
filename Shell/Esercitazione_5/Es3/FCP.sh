#!/bin/sh
#Script che controlla:
#1. Che siano passati almeno 3 parametri
#2. Che i primi N siano directory passate in nome assoluto
#3. Che l'ultimo parametro sia una stringa

#Controllo che sia passato il numero giusto di parametri
case $# in
0|1|2)	
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)	
	;;
esac

#Creo la variabile list per salvare i primi N nomi assoluti e per salvare la stringa S
S=
list=
#Creo la variabile count per tenere conto a che parametro siamo arrivati
count=0

#scorro i vari parametri passati
for i
do
	count=`expr $count + 1`
	#controllo che non sia l'ultimo parametro
	if test $count -ne $#
	then
		#controllo che il nome sia assoluto
		case $i in
		/*)	
			#controllo che sia una directory traversabile
			if test ! -d $i -o ! -x $i
			then
				echo "ERRORE: $i non directory o no traversabile"
				exit 2
			else
				list="$list $i"
			fi;;
		*)	
			echo "ERRORE: $i non è un nome assoluto"
			exit 3;;
		esac
	else
		#controllo che non sia contenuto il carattere /
		case $i in
		*/*)	
			echo "ERRORE: $i contiene il carattere /"
			exit 4;;
		*)	
			S=$i;;
		esac
	fi
done

#Aggiorno la variabile PATH e la esporto
PATH=`pwd`:$PATH
export PATH

#creo un file temporaneo per tenere i nomi dei file
> /tmp/conta$$

#esploro le varie gerarchie
for i in $list
do
	echo DEBUG-esploro la gerarchia $i
	#invoco il file ricorsivo
	FCR.sh $i $S /tmp/conta$$
done

#definisco la variabile c che conta a che punto dello stream di /tmp/conta$$
c=0

for i in `cat /tmp/conta$$`
do
	echo DEBUG-valuto se sono sul numero di caratteri o sul nome assoluto
	c=`expr $c + 1`
	#verifico se l'elemento corrente è il numero di caratteri o il nome assoluto
	if test `expr $c % 2` -eq 1
	then
		echo "Numero di caratteri: $i"
	else
		echo "Nome assoluto del file: $i"
		echo
		echo "Vuoi ordinare il file in ordine alfabetico ordinario e inverso? (sì/no)"
		read RISPOSTA
		case $RISPOSTA in
		sì)
			echo ===
			sort -f $i
			echo ===
			sort -r $i
			echo ===;;
		no)	
			echo "Hai deciso di non ordinare il file $i";;
		esac
	fi
done

#elimino il file temporaneo
rm /tmp/conta$$
