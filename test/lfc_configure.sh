#!/bin/bash
## script to an auto configuration of a dpm storage element for the srm unit test
#
echo "$1"
if [ ["$1" = ""] -o ["$2" = ""] ]; then
	echo "Invalid Usage"
	echo "	$0 LFC_PREFIX SRM_ENDPOINT ...."
	exit -1
fi


function create_directory {
	lfc-mkdir $1
}

function change_right {
	lfc-chmod $1 $2
}

TESTOPENDIR0011="$1/testopendir0011"
TESTOPENDIR0012="$1/testopendir0012"

create_directory "$TESTOPENDIR0011"
create_directory "$TESTOPENDIR0012"
change_right 000 "$TESTOPENDIR0012"
