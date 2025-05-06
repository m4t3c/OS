#!/bin/sh
#Script che controlla ricorsivamente se nella directory corrente è presente uno o più file con almeno $2 righe
#successivamente salva il nome assoluto del file in $3

cd $1

#Definisco la variabile NL per salvare al suo interno il numero di linee
NL=

for F in *
do
	#Controllo che l'elemento sia un file e sia leggibile
	if test -f $F -a -r $F
	then
		NL=`wc -l < $F`
		
		#Controllo che NL sia maggiore o uguale di $2
		if test $NL -ge $2
		then
			echo $1/$F >> $3
		fi
	fi
done

for i in *
do
	if test -d $i -a -x $i
	then
		#Chiamo ricorsivamente sulla sottodirectory $i
		$0 $1/$i $2 $3
	fi
done
