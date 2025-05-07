#!/bin/sh
#File principale che fa:
#1. Il controllo dei parametri
#2. L'invocazione del file ricorsivo
#3. La stampa del nome assoluto della gerarchia seguito dal numero totale di file trovati che soddisfano le caratteristiche
#4. La stampa del nome assoluto di ogni file trovato
#5. La richiesta per ogni file trovato all'utente di un numero X che serve per stampare le X righe del file

#Controllo che siano almeno due parametri
case $# in
0|1)
	echo "ERRORE: hai passato $# paramentri mentre ne sono richiesti almeno 2"
	echo "Usage is: $0 dirass1 dirass2 ..."
	exit 1;;
*)
	;;
esac

#Controllo che i parametri siano nomi assoluti di directory traversabili
for G
do
	case $G in
	/*)
		if test ! -d $G -o ! -x $G
		then
			echo "ERRORE: $G non directory o non traversabile"
			exit 2
		fi;;
	*)
		echo "ERRORE: $G non e' un nome assoluto"
		exit 3;;
	esac
done

#Creo la variabile n che serve per creare i file temporanei diversi per ogni gerarchia G
n=0

#Aggiorno la variabile PATH e la esporto per compatibilità
PATH=`pwd`:$PATH
export PATH

#Invoco lo script ricorsivo per ogni gerarchia
for i
do
	n=`expr $n + 1`
	#Creo il file temporaneo
	> /tmp/nomiAssoluti-$n
	FCR.sh $i /tmp/nomiAssoluti-$n
done

#riporto a 0 la variabile n per poter esplorare tutti i file temporanei
n=0

#Scorro tutte le gerarchie
for G
do
	#Incremento di 1 la variabile n
	n=`expr $n + 1`
	#Stampo il numero di file trovati per la gerarchia G
	echo "---"
	echo "Numero di file trovati nella gerarchia $G: `wc -l < /tmp/nomiAssoluti-$n`"
	#Creo un for che scorra tutti i file della gerarchia salvati nel file temporaneo
	for f in `cat /tmp/nomiAssoluti-$n`
	do
		echo "File che rispetta le condizioni: $f"
		echo -n "Quante righe del file $f vuoi leggere? " > /dev/tty
		read X
		#Conrtollo che il numero sia strettamente positivo
		case $X in
		*[!0-9]*)
				echo "ERRORE: il parametro passato: $X non e' valido, pertanto non verra' stampato nulla";;
		*)		
				if test $X -eq 0
				then
					echo "ERRORE: il numero passato non e' diverso da 0"
				
				else
					echo "==="
					head -$X $f
					echo "==="
				fi;;
		esac
	done
	echo "---"
done

