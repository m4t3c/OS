#!/bin/sh
#File comandi ricorsivo che cerca se nella directory corrente è presente un file che si chiami $2 e che esegue i seguenti comandi:
#1. Crea un file ordinato con lo stesso contenuto del file F
#2. Passa il nome assoluto del file ordinato al file temporaneo passato come terzo parametro

#Mi sposto nella directory passata come primo parametro
echo DEBUG-entro nella directory $1
cd $1

#Controllo se c'è un file che si chiama $2
echo DEBUG controllo ci sia $2 in `ls $1`
if test -f $2 -a -r $2
then
	sort -f $2 > sorted	#ordino senza tener conto di maiuscole e minuscole con -f e reindirizzo su sorted
	echo $1/sorted >> $3	#Salvo il nome assoluto del file sorted nel file temporaneo
fi

#Richiamo ricorsivamente il file in una eventuale sottodirectory per continuare l'esplorazione della gerarchia
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done

