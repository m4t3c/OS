#!/bin/sh
#Cerchiamo il file $2 in $1
cd $1

if test -f $2
then
	echo "Il file si trova in `pwd`"
fi

for i in *
do
	if test -d $i -a -x $i
	then
		$0 $1/$i $2
	fi
done
