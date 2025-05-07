#!/bin/sh
#  $0	  $1	  $2	  $3	  $4	  $5
# FCR.sh dir	   X    livello	 tmp1    tmp2

#Ci spostiamo nella directory
cd $1

#Creo una variabile livello prendendo il terzo parametro e incremento di 1
livello=`expr $3 + 1`

#Se ci troviamo nel livello richiesto salvo il nome assoluto della directory nel file tmp1
if test $livello -eq $2
then
	echo $1 >> $4
	#Scorro ogni elemento della directory corrente
	for F in *
	do
		#Controllo che l'elemento corrente sia un file e sia leggibile
		if test -f $F -a -r $F
		then
			#Creo una variabile NC per salvare i caratteri del file
			NC=`wc -c < $F`
			#controllo che NC non sia 0
			if test $NC -ne 0
			then
			echo $F >> $5
			fi
		fi
	done
fi

#Chiamo ricorsivamente lo script per le altre gerarchie
for G in *
do
	if test -d $G -a -x $G
	then
		$0 $1/$G $2 $livello $4 $5
	fi
done

