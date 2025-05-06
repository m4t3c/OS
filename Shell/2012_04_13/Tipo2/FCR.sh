#!/bin/sh
#Script ricorsivo che:
#nella prima fase conta il livello massimo
#nella seconda fase si cercano tutte le directory del livello richiesto dall'utente

#Mi sposto nella directory corrente
cd $1

#incremento il livello corrente
livello=`expr $2 + 1`

#Controllo in che fase sono
if test $3 -eq 1
then
	#Se sono nella prima fase controllo se il livello corrente è più grande di quello massimo registrato sul file temporaneo
	prec=`cat $4`
	#Controllo che il livello corrente sia più grande di quello massimo
	if test $livello -gt $prec
	then
		#salviamo il livello corrente nel file temporaneo
		$livello > $4
	fi
else
	#Se siamo nella seconda fase dobbiamo controllare se siamo nella gerarchia specificata
	#Nel caso stampare tutte le informazioni degli elementi contenuti nelle directory
	if test $livello -eq $4
	then
		ls -la
	fi
fi

#Invoco ricorsivamente lo script
for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $livello $3 $4
	fi
done

