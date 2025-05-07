#!/bin/sh
#  $0	 $1	$2
#FCR.sh dirass  tmp

#Mi sposto nella directory corrente ($1)
cd $1

#Creo una variabile count per contare in che posizione sono dell'esplorazione della directory
count=0
for F in *
do
	#Controllo che $F sia un file
	if test -f $F
	then
		#incremento la variabile count
		count=`$count + 1`
		#Se sono nella terza posizione allora salvo il suo nome assoluto in tmp ($2)
		if test $count -eq 3
		then
			$1/$F >> $2
			break
		fi
	fi
done

#Nel caso non ci sia un file in terza posizione riporto il nome assoluto della directory con spiegazione
if test $count -ne 3
then
	echo "Nella directory $1 non ci sono 3 file, ce ne sono $conta"
fi

#Invoco ricorsivamente lo script ricorsivo per le sottodirectory
for G in *
do
	if test -d $G -a -x $G
	then
		$0 $1/$G $2
	fi
done
