#!/bin/bash



file_list=( "http://https-server.cern.ch/group.test.hc.NTUP_SMWZ.root" "root://xrd-test:1094//xrd/group.test.hc.NTUP_SMWZ.root" 
	    "https://se2.ppgrid1.rhul.ac.uk/dpm/ppgrid1.rhul.ac.uk/home/dteam/group.test.hc.NTUP_SMWZ.root"  "root://se2.ppgrid1.rhul.ac.uk//dteam/group.test.hc.NTUP_SMWZ.root"             "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/group.test.hc.NTUP_SMWZ.root" "root://lxfsra04a04.cern.ch//dteam/group.test.hc.NTUP_SMWZ.root" 
	    "https://lcgse0.shef.ac.uk/dpm/shef.ac.uk/home/dteam/group.test.hc.NTUP_SMWZ.root" "root://lcgse0.shef.ac.uk//dteam/group.test.hc.NTUP_SMWZ.root"
	    "https://gridpp09.ecdf.ed.ac.uk/dpm/ecdf.ed.ac.uk/home/dteam/group.test.hc.NTUP_SMWZ.root" "root://gridpp09.ecdf.ed.ac.uk//dteam/group.test.hc.NTUP_SMWZ.root"
	    "https://t2-dpm-01.na.infn.it/dpm/na.infn.it/home/dteam/group.test.hc.NTUP_SMWZ.root" "root://t2-dpm-01.na.infn.it:1094//dteam/group.test.hc.NTUP_SMWZ.root")
prefix_list=("http" "xroot" "http" "xroot" "http" "xroot" "http" "xroot" "http" "xroot" "http" "xroot")
site_list=("STANDALONE" "STANDALONE" "RHUL" "RHUL" "CERN" "CERN" "SHEF" "SHEF" "ECDF" "ECDF" "NA-INFN" "NA-INFN") 


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
	./testHammercloud ${file_list[${i}]} &> $1/${prefix_list[${i}]}-${site_list[${i}]}-$RANDOM-`date +%s`

	echo "end execution"
done


done

