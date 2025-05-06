#!/bin/sh
#Script ricorsivo che fa:
#1. lo spostamento sulla directory corrente
#2. Controlla se è presente un file con terminazione .$2 e in tal caso scrive il nome assoluto della directory su $3
#3. Continua ad esplorare la gerarchia ricorsivamente

#Mi sposto nella directory corrente
cd $1

#Scorro gli elementi della directory corrente
for F in *
do
	#controllo che l'elemento abbia terminazione .$2
	case $F in
	*.$2)
		if test -f $F
		then
			echo "$1" >> $3
			break
		fi;;
	*)
		;;
	esac
done

#Invoco ricorsivamente FCR.sh per le directory traversabili
for G in *
do
	if test -d $G -a -x $G
	then
		$0 $1/$G $2 $3
	fi
done
