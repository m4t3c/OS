#!/bin/sh
#Script ricorsivo che esegue le seguenti operazioni
#1. Controlla che la directory corrente si chiami come $2
#2. Controlla che al suo interno ci sia un file con un numero al suo interno
#3. Salva il nome assoluto della directory in $3

#Mi sposto sulla directory passata come primo parametro
cd $1

#Definisco una variabile che mi dica se ho trovato una directory valida
trovata=false

#Controllo che la directory corrente termini con $2
case $1 in
*/$2)
	#Scorro tutti gli elementi della directory per cercare un file che contenga un numero
	for i in *
	do
		#controllo che l'elemento iesimo sia un file
		if test -f $i
		then
			if grep '[0-9]' $i > /dev/null 2>&1
			then
				#in caso trovo un numero in un nome salvo il nome assoluto della directory nel terzo parametro
				echo $1/$i >> $3
				trovato=true
			fi
		fi
	done;;
*)
	;;
esac

#Stampo il nome assoluto della directory
if test "$trovato" = true
then
	echo "TROVATA DIRECTORY VALIDA: $1"
fi

#Invoco ricorsivamente lo script su tutte le directory
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done
