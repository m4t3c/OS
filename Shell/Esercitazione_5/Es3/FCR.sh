#!/bin/sh
#Script che esplora la gerarchia passata e salva il nome assoluto di tutti i file che iniziano con la stringa S

cd $1

#Creo una variabile per salvare i caratteri del file
lunghezza=

if test -f $2.txt -a -r $2.txt -a -w $2.txt
then
	echo DEBUG-ho trovato il file corretto
	lunghezza=$(wc -c < $2.txt)
	echo $lunghezza `pwd`/$2.txt >> $3 
fi

for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done

