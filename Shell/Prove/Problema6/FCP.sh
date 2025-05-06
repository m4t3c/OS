#!/bin/sh
#Script che consente di:
#1. Controllare che siano passati almeno 2 parametri
#2. Controllare che tutti i parametri siano nomi assoluti di directory traversabili
#3. Invocare lo script ricorsivo FCR.sh
#4. Stampare su standard output il nome assoluto di ogni file trovato che soddisfa le specifiche
#5. Chiedere all'utente un numero X e in seguito stampare X righe del file corrente

#Controllo i parametri
case $# in
0|1)
	echo "ERRORE: hai passato $# parametri mentre ne sono richiesti almeno 2 - Usage is $0 nomassdir1 nomassdir2 ... nomassdirn"
	exit 1;;
*)
	;;
esac

#Controllo che tutti i parametri siano nomi assoluti di directory traversabili
for i
do
	case $i in
	/*)
		if test ! -d $i -o ! -x $i
		then
			echo "ERRORE: $i non directory o non traversabile"
			exit 2
		fi;;
	*)
		echo "ERRORE: $i non in nome assoluto"
		exit 3;;
	esac
done

#Aggiorno la variabile PATH
PATH=`pwd`:$PATH
export PATH

#definisco il counter n
n=1

for i
do
	#Creo file temporaneo per ogni processo
	> /tmp/conta$$-$n
	#Faccio la chiamata ricorsiva
	./FCR.sh $i /tmp/conta$$-$n
	
	#incremento n
	n=`expr $n + 1`
done

#ripristino a 1 la variabile n per poter ripartire dal primo file temporaneo
n=1

#Per ogni file temporaneo stampo il nome assoluto di ogni file e chiedo quante X righe visualizzare
for i
do
	echo "Numero totale di file trovati nella gerarchia $i: `wc -l < /tmp/conta$$-$n/`"
	for j in `cat /tmp/conta$$-$n`
	do
		echo "Nome assoluto del file: $j"
		echo
		echo -n "Quante righe vuoi visualizzare? "
		read X
		#Controllo che X sia numerico
		case $X in
		*[!0-9]*)
				echo "ERRORE: $X non è un numero"
				continue;;
		*)
				;;
		esac
		echo ===
		head -$X < $j
		echo ===
	done
	n=`expr $n + 1`
done

#cancello i file temporanei
rm /tmp/conta$$-*

