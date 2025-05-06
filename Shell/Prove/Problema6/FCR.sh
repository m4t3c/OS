#!/bin/sh
#Script che esplora la directory passata con $1 e salva il nome assoluto dei file che hanno al loro interno tutte le righe che iniziano per a
#Successivamente continua ad esplorare la gerarchia sottostante

#Mi sposto nella directory corrente
cd $1

#Definisco due variabili: una conta il numero di linee globali mentre l'altra quante linee iniziano con a
NR=
NG=

#Scorro tutti gli elementi della directory
for i in *
do
	#controllo che l'elemento sia un file leggibile
	if test -f $i -a -r $i
	then
		#conto le linee totali del file
		NR=`wc -l < $i`
		#Controllo che le linee siano diverse da 0
		if test $NR -ne 0
		then
			#Salvo il numero di linee che iniziano per a
			NG=`grep '^a' $i | wc -l`
			if test $NR -eq $NG
			then
				echo `pwd`/$i >> $2
			fi
		fi
	fi
done

#Continuo l'esplorazione
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2
	fi
done
