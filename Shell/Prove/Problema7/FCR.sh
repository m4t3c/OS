#!/bin/sh
#Script che esegue la fase A e B
#FASE A: Conta tutti i livelli della gerarchia G
#FASE B: Accede alla gerarchia data dall'utente e stampa le informazioni di tutti i file della directory

#Mi sposto alla directory corrente
cd $1

#incremento il valore della variabile locale conta
conta=`$2 + 1`

for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $conta $3 $4
		ret=$?
		re
