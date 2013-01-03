#!/bin/bash 


NPROC=$(cat tests/00-scenario1.txt | head -1)

for i in `seq 1 $NPROC`;
do
	./test_prog &
done

rm -fr tmp/00scenario1 > /dev/null
mkdir tmp/00scenario1 -p

WORKDIR=`pwd`/tmp/00scenario1
PROCS=""

for i in `ps ax | grep test_prog | tail -n $NPROC | awk '{print $1}'`; do PROCS="$PROCS $i"; done

echo $PROCS > $WORKDIR/procs.txt
LICZNIK=0
for i in `cat tests/00-scenario1.txt | awk '{ if (NR!=1) { print $0}}'`;
do
	LICZNIK=$(($LICZNIK+1))
	PROC=$(echo $i | awk 'BEGIN {FS="."} { print $1}')
	OP=$(echo $i | awk 'BEGIN {FS="."} { print $2}')
	mkdir $WORKDIR/$LICZNIK
	nPROC=$(echo $PROCS | awk "{print $PROC}")
	./prog_lag $nPROC $OP
	sleep 2
	dmesg > $WORKDIR/$LICZNIK/dmesg.txt
done

killall test_prog