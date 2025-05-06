#!/bin/sh
#Script che ricorsivamente esplora tutte le N gerarchie passate e che per ogni file:
#1. Controlla che abbia almeno Y righe
#2. Per ogni file crea un file di nome f.quinta se riesce ad accedere alla quinta riga sennò ne crea uno vuoto di nome f.NOquinta
#3. riporta su standard output il numero totale di file creati globalmente, il nome assoluto di ogni file creato e il contenuto del file

NL=	#variabile in cui salvo il numero di linee correnti

#mi sposto sulla directory $1
echo DEBUG-accedo alla directory $1
cd $1


#con un for esploro tutti i file contenuti dalla directory
for i in *
do
	#controllo che siano file leggibili
	if test -f $i -a -r $i
	then
		#Calcolo il numero di linee del file corrente
		NL=`wc -l < $i`
		#controllo che il file i contenga almeno Y righe
		if test $NL -ge $2
		then
			if test $NL -lt 5
			then
				> $i.NOquinta
				echo `pwd`/$i.NOquinta >> $3
			else
				(head -5 $i | head -1) > $i.quinta
				echo `pwd`/$i.quinta >> $3
			fi
		fi
	fi
done	

for i in *
do
	if test -d $i -a -x $i
	then
		$0 `pwd`/$i $2 $3
	fi
done

