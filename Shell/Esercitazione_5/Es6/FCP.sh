#!/bin/sh
#Script che controlla che:
#1. Siano passati almeno 3 parametri
#2. Il primo parametro sia un numero intero strettamente positivo
#3. Gli altri parametri siano nomi assoluti di directory

#Definisco la variabile per il primo parametro
X=

#Controllo che i parametri siano almeno 3
case $# in
0|1|2)	
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 3"
	exit 1;;
*)
	;;
esac

#Controllo che il primo parametro sia strettamente positivo
case $1 in
*[!0-9]*)
		echo "ERRORE: il numero passato non è un numero"
		exit 2;;
*)		
		if test $i -eq 0
		then
			echo "ERRORE: numero passato uguale a 0"
			exit 3
		fi;;
esac

#Arrivato a questo punto posso salvare il primo parametro in X e fare uno shift
X=$1
shift

#Scorro i parametri rimanenti e controllo che siano nomi assoluti di directory traversabili
for Q
do
	case $Q in
	/*)	
		if test ! -d $Q -o ! -x $Q
		then
			echo "ERRORE: nome non directory o non traversabile"
			exit 4
		fi;;
	*)
		echo "ERRORE: parametro non in nome assoluto"
		exit 5;;
	esac
done

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#Creo la variabile F per avere un file diverso per ogni gerarchia
n=1

#Invoco ricorsivamente il file FCR.sh
for $G
do
	#Creo il file temporaneo dove salvare i file temporanei
	> /tmp/nomiAssoluti-$n

	FCR.sh $G $X /tmp/nomiAssoluti-$n
	echo "Numero di linee trovato è `wc -l < /tmp/nomiAssoluti-$n`"
	#Incremento la variabile n per poter differenziare il file temporaneo successivamente
	n=`expr $n + 1`
done

#io quì farei due for concatenati uno che scorre l'iesimo (con i > 1) e l'altro che scorre il primo file
#se si trovano due elementi uguali li si stampa
for file1 in `cat /tmp/nomiAssoluti-1`
do
	n=1
	for i 
	do
		if test $n -gt 1
		then
			for file2 in `cat /tmp/nomiassoluti-$n`
			do
				if diff $file1 $file2 > /dev/null 2>&1			#con il comando diff posso vedere se due file sono uguali
				then
					echo "I file $file1 e $file2 sono uguali"
				fi
			done
		fi
		n=`expr $n + 1`
	done
done

#Cancello tutti i file temporanei
rm /tmp/nomiAssoluti-*
