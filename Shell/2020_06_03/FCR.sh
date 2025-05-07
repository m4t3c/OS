#!/bin/sh
#  $0	  $1	$2	$3
# FCR.sh  $D	$C      tmp

#Mi sposto nella directory passata come primo parametro
cd $1

#Controllo che la directory abbia il nome che soddisfa le specifiche richieste
case $1 in
	*$2?$2)
		pwd >> $3;;
	*)
		;;
esac

for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done
