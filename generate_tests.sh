#/bin/bash
##
#
test_list=`grep -v "//" test/unit_test_cgreen.c  | grep -E "add_test(.*)" | sed "s/.*add_test(.*, \(.*\));/\1/g"`

test_script_dir=`dirname $0`/dist/usr/share/gfal2/tests/mocked

for i in $test_list
do
echo "#/bin/bash" > $test_script_dir/$i
echo "\`dirname \$0\`/test_verbose $i" >> $test_script_dir/$i
done
