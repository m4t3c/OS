#!/bin/sh
#Script che esplora la directory passata e se si trova un file lo salva in $2

cd $1

for i in *
do
	if test -f $i
	then
		echo $1/$i >> $2
	fi
done

#invoco la chiamata ricorsiva
for i in *
do
	#controllo che sia una directory traversabile
	if test -d $i -a -x $i
	then
		$0 $1/$i $2
	fi
done

