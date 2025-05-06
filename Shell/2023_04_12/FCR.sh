#!/bin/sh
#Script ricorsivo che fa:
#1. lo spostamento sulla directory passata come primo parametro
#2. Lo scorrimento di tutti gli elementi della directory passata
#3. Il controllo se l'elemento corrente è un file e termina con $2 o $3 e in tal caso scrive il suo nome assoluto in $4 o $5 rispettivamente
#4. Continua l'esplorazione della gerarchia nelle eventuali sottodirectory

#Mi sposto nella directory corrente
cd $1

#Scorro tutti i parametri
for F in *
do
	#Controllo che l'elemento sia un file leggibile
	if test -f $F -a -r $F
	then
		#Controllo che il file termini con S1 o S2 (param 2 e 3)
		case $F in
		*.$2)
			echo "$1/$F" >> $4;;
		*.$3)
			echo "$1/$F" >> $5;;
		*)
			;;
		esac
	fi
done

#Invoco ricorsivamente lo script per esplorare le sottodirectory
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3 $4 $5
	fi
done
