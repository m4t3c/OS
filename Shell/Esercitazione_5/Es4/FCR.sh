#!/bin/sh
#Script che esplora la gerarchia di ciascuna delle N directory e controlla che
#1) il nome sia composto da 3 caratteri
#2) i caratteri dispari siano uguali al carattere C

#ci spostiamo nella directory passata come primo parametro
cd $1

case $1 in
*/$2?$2) 
	echo DEBUG-Directory che soddisfa i requisiti, la aggiungo al file temporaneo
	$1 >> $3;;
*)	;;
esac

for i in *
do
	if test -d $i -a -x $i
	then	
		$0 `pwd`/$i $2 $3	#invoco ricorsivamente il file entrando nella directory i
	fi
done

