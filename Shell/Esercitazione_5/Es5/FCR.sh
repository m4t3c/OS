#!/bin/sh
#Script che controlla se nella directory passata come primo parametro è contenuto un file con terminazione .S e inserisce nel file temporaneo il nome assoluto della directory

#Mi trasferisco nella directory $1
cd $1

#Definisco la variabile trovato
trovato=false

#Controllo se c'è un file nella directory con terminazione .S
for i in *
do
	if test -f $i
	then
		case $i in
		*.$2)
			#in questo caso i vincoli sono rispettati, imposto la variabile trovato a true e faccio un break
			trovato=true
			break;;
		*)
			;;
		esac
	fi
done

#se trovato è true allora devo scrivere il nome assoluto della directory nel file temporaneo
if test $trovato = true
then
	$1 >> $3
fi

#invoco ricorsivamente il file FCR.sh per esplorare le gerarchie
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done
