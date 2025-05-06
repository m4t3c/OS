#!/bin/sh
#Script che esplora una gerarchia e che conta le directory che soddisfano le seguenti specifiche:
#1. contengono solo file
#2. il numero di linee di ogni file è strettamente maggiore di X

#mi posiziono nella directory data dal parametro $1
cd $1

#definisco le variabili di controllo che mi serviranno
isdir=0
isenough=1
righe=

#controllo che ci siano solo file nella directory
for i in *
do
	if test ! -f $i
	then	
		isdir=1
		break
	fi
done

#se isdir è rimasto a 0 proseguiamo con i controlli sennò si invoca la chiamata ricorsiva
if test $isdir -eq 0
then
	#controllo che tutti i file abbiano almeno X linee
	for i in *
	do	
		righe=$(wc -l < $i)
		if test $righe -lt $2
		then
			isenough=0
			break
		fi
	done
	if test $isenough -eq 1
	then
		echo "Trovata directory: $1"
		echo
		for i in *
		do
			echo "File: $1/$i"
			echo "riga numero $2 dalla fine: $(tail -n $2 $i | head -n 1)"
			echo "---"
		done
		echo
	fi
fi

for i in *
do
	if test -d $i -a -x $i
	then
		$0 "$1/$i" $2
	fi
done

