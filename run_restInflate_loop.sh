#!/bin/bash



file_list=( "http://https-server.cern.ch/h1big.root" "root://xrd-test:1094//xrd/h1big.root" )
prefix_list=("http" "xrootd")

if [[ "$1" == "" ]]; then
	echo "Usage $0 [dir]"
	exit -1
fi


mkdir -p $1

while [ true ]
do


for ((i=1; i<= ${#file_list[@]}; i++))
do
	echo "execute testInflate on ${file_list[${i}]}"
	./testInflate ${file_list[${i}]} &> $1/${prefix_list[${i}]}-$RANDOM-`date +%s`

	echo "end execution"
done


done

