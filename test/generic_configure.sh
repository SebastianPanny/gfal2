#!/bin/bash
## script to an auto configuration for the unit tests, with gfalFS
#

TEST_SRM_ENDPOINT="srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/"
TEST_LFC_ENDPOINT="lfn:/grid/dteam/"

TEST_SRM_FILE_CONTENT="Hello world"

## srm vars
TEST_SRM_ONLY_READ_ACCESS="testread0011"
TEST_SRM_ONLY_READ_HELLO="testreadhello001"
TEST_SRM_NOEXIST_ACCESS=$TEST_SRM_INVALID_SURL_EXAMPLE2
TEST_SRM_NO_READ_ACCESS="testnoread0011"
TEST_SRM_WRITE_ACCESS="testwrite0011"
TEST_SRM_NO_WRITE_ACCESS="testnowrite0011"

TEST_SRM_CHMOD_FILE_EXIST="testchmod0011"
TEST_SRM_CHMOD_FILE_ENOENT="testchmodenoent0011"

TEST_GFAL_SRM_FILE_STAT_OK="teststat0011"

TEST_LFC_VALID_COMMENT="testcomment0011"
TEST_LFC_WRITEVALID_COMMENT="testcomment0012"
TEST_LFC_INVALID_COMMENT="testpsg0011"
TEST_LFC_COMMENT_CONTENT="Hello World"

## lfc vars
TEST_LFC_ONLY_READ_HELLO="hello001"
TEST_LFC_ONLY_READ_ACCESS="testread0011"
TEST_LFC_NO_READ_ACCESS="testnoread0011"
TEST_LFC_WRITE_ACCESS="testwrite0011"	
TEST_LFC_NO_WRITE_ACCESS="testnowrite0011"

# write files 
function create_on_grid {
	echo "$1" > $2
}

function create_directory {
	mkdir $1
}

function change_right {
	chmod $1 $2
}

function create_stat_basic {
	dd if=/dev/urandom of=/tmp/teststattmp bs=512 count=4;
	cp /tmp/teststattmp $1;
	chmod 664 $1;
}

function set_comment_on_file {
	setfattr -n "user.comment" -v "$1" $2
}

## CONFIGURE SRM ENV
#
mkdir -p /tmp/mount_point_gfal
gfalFS  /tmp/mount_point_gfal $TEST_SRM_ENDPOINT
cd /tmp/mount_point_gfal

# main files for access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_ONLY_READ_ACCESS
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_ONLY_READ_HELLO
# created but impossible to access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_NO_READ_ACCESS
change_right 000 $TEST_SRM_NO_READ_ACCESS
# main file for write access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_WRITE_ACCESS
change_right 777 $TEST_SRM_WRITE_ACCESS
# main file for no write access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_NO_WRITE_ACCESS
change_right 555 $TEST_SRM_NO_WRITE_ACCESS
# main filefor chmod
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_CHMOD_FILE_EXIST

## main file for stats calls
create_stat_basic "$TEST_GFAL_SRM_FILE_STAT_OK"

## create comments files
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_LFC_VALID_COMMENT
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_LFC_WRITEVALID_COMMENT

cd ~/
gfalFS_umount  /tmp/mount_point_gfal


### CONFIGURE LFC ENV
## FILES CAN NOT BE CREATED ON LFC CURRENTLY


mkdir -p /tmp/mount_point_gfal
gfalFS  /tmp/mount_point_gfal $TEST_LFC_ENDPOINT
cd /tmp/mount_point_gfal

## lfc access
change_right 000 $TEST_LFC_NO_READ_ACCESS
change_right 777 $TEST_LFC_WRITE_ACCESS
change_right 555 $TEST_LFC_NO_WRITE_ACCESS

# comments
set_comment_on_file "$TEST_LFC_COMMENT_CONTENT" $TEST_LFC_VALID_COMMENT

cd ~/
gfalFS_umount  /tmp/mount_point_gfal
