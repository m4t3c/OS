#!/bin/sh
#  $0	  $1	  $2	 $3
# FCR.sh dirass   $F    tmp

#Mi sposto nella directory corrente
cd $1

#Controllo se c'è un file leggibile che si chiami F
if test -f $2 -a -r $2
then
	#Lo riordino ignorando le maiscuole e le minuscole e salvo nel file sort
	sort -f $2 > sorted
	#metto il file sorted nel file tmp
	echo $1/sorted >> $3
fi

#Continuo ad esplorare le gerarchie
for G in *
do
	if test -d $G -a -x $G
	then
		$0 $1/$G $2 $3
	fi
done	
