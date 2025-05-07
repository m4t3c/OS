#!/bin/sh
#Script ricorsivo
#  $0	  $1	 $2	$3
#FCR.sh dirass	  X	tmp

#Mi sposto nella directory corrente
cd $1

#Creo la variabile che contiene il numero di righe del file esaminato
NL=


#Scorro tutti gli elementi della directory corrente
for F in *
do
	#Controllo che l'elemento sia un file e che sia leggibile
	if test -f $F -a -r $F
	then
		#Salvo il numero di linee nella variabile NL
		NL=`wc -l < $F`
		#Controllo che NL sia uguale a X ($2), nel caso scrivo il nome assoluto del file in $3
		if test $NL -eq $2
		then
			echo $1/$F >> $3
		fi
	fi
done

#Invoco ricorsivamente il file ricorsivo per continuare ad esplorare la gerarchia
for $Q in *
do
	if test -d $Q -a -x $Q
	then
		$0 $1/$Q $2 $3
	fi
done
