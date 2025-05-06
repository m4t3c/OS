#!/bin/sh
#Script che conta quanti file hanno H righe e che contengono il carattere Cx
#Nel caso il numero contato di file sia N viene stampato su standard output il nome assoluto della directory

#mi trasferisco nella directory corrente
cd $1

#Definisco la variabile count che conta quanti file hanno H righe e contengono il carattere Cx
count=0

#Scorro tutti gli elementi della directory
for i in *
do
	#Controllo che il numero di righe sia giusto
	if test `wc -l < $i` -eq $2
	then
		#Controllo che sia contenuto il carattere $4
		if grep $4 $i > /dev/null 2>&1
		then
			count=`expr $count + 1`
		fi
	fi
done

#Se il numero di file individuati è pari a $3 allora stampo il nome della directory
if $count -eq $3
then
	echo "Directory trovata: $1"
fi

#Invoco ricorsivamente lo script per esplorare la gerarchia G
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3 $4
	fi
done
