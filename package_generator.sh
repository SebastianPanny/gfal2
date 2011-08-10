#!/bin/bash



package_list=(main main_devel doc meta dcap srm lfc rfio)

mkdir -p RPMS
for i in ${package_list[@]}
do
scons -j 8 package=$i "$@"
if [ "$?" != "0" ]
  then
	exit $?
fi
rm *.tar.gz
rm *.spec
mv *.rpm RPMS/
done

