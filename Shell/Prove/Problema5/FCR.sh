#!/bin/sh
#Script che controlla se nella directory corrente è presente almeno un file con estensione E1 e almeno uno con estensione E2
#In caso affermativo stampa su standard output il nome assoluto della directory
#Successivamente chiama ricorsivamente lo script FCR.sh

#Mi sposto sulla directory corrente
cd $1

#Definisco due variabili E1 ed E2 per fare il controllo che sia contenuto un file di entrambi
E1=false
E2=false

#Scorro tutti i file della directory
for i in *
do
	case $i in
	*$2)
		E1=true;;
	*$3)
		E2=true;;
	*)
		;;
	esac
done

if $E1 = "true" -a $E2 = "true"
then
	echo "Directory trovata: $1"
fi

for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2 $3
	fi
done

