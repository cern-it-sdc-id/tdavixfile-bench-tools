#!/bin/bash



file_list=( "http://https-server.cern.ch/group.test.hc.NTUP_SMWZ.root" "root://xrd-test:1094//xrd/group.test.hc.NTUP_SMWZ.root" )
prefix_list=("http" "xrootd")

if [[ "$1" == "" ]]; then
	echo "Usage $0 [dir]"
	exit -1
fi


mkdir -p $1

while [ true ]
do


for ((i=0; i< ${#file_list[@]}; i++))
do
	echo "execute testHammerCloud  on ${file_list[${i}]}"
	./testHammercloud ${file_list[${i}]} &> $1/${prefix_list[${i}]}-$RANDOM-`date +%s`

	echo "end execution"
done


done

