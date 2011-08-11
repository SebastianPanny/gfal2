#!/bin/bash

package_list=(main main_devel doc meta dcap srm lfc rfio)

if [ -z "$SCONS_EXE" ]
then
	SCONS_EXE="scons"
fi


mkdir -p RPMS
for i in ${package_list[@]}
do
$SCONS_EXE -j 8 package=$i "$@"
if [ "$?" != "0" ]
  then
	exit $?
fi
rm *.tar.gz
rm *.spec
mv *.rpm RPMS/
done

