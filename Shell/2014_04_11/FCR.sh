#!/bin/sh
#  $0		$1	$2
# FCR.sh	dir	tmp

#Mi sposto nella directory passata come parametro 
cd $1

#Scorro tutti gli elementi della directory
for F in *
do
	#Controllo che l'elemnto sia un file e che sia leggibile
	if test -f $F -a -r $F
	then
		#Salvo il numero di righe del file nella variabile NR
		NR=`wc -l < $F`
		if test $NR -ne 0
		then
		#Salvo il numero di righe che iniziano con a in NA
		NA=`grep -v '^a' $F | wc -l`
		#Controllo che NR e NA siano uguali
			if test $NR = $NA
			then
				#Salvo il nome assoluto del file nel file temporaneo
				echo $1/$F >> $2
			fi
		fi

	fi
done

#Chiamo ricorsivamente lo script per continuare l'esplorazione della gerarchia
for G in *
do
	if test -d $G -a -x $G
	then
		$0 $1/$G $2
	fi
done
