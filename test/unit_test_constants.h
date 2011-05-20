#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 
 // constant to define for succefull unit test
#define TEST_LFC_VALID_ACCESS "lfn:/grid/dteam/hello001"		// this file must be a lfc file with read access and no write access
#define TEST_LFC_ONLY_READ_ACCESS "lfn:/grid/dteam/testread0011"
#define TEST_LFC_NOEXIST_ACCESS "lfn:/grid/dteam/PSGmarqueUNbut" // this file must never exist.
#define TEST_LFC_NO_READ_ACCESS "lfn:/grid/dteam/testnoread0011" // this file is un-accessible be read()
#define TEST_LFC_WRITE_ACCESS "lfn:/grid/dteam/testwrite0011"	// must be writed
#define TEST_LFC_NO_WRITE_ACCESS "lfn:/grid/dteam/testnowrite0011"
#define TEST_LFC_URL_SYNTAX_ERROR "jardiland.com"

// GUID constants
#define TEST_GUID_VALID_ACCESS "guid:ae571282-81ea-41af-ac3c-8b1a084bfc82"
#define TEST_GUID_ONLY_READ_ACCESS "guid:c8b5ed56-1653-4de6-8303-9ecf3c510da8"
#define TEST_GUID_WRITE_ACCESS "guid:e15f0bce-813e-44d1-98c6-c3501d5fa707"
#define TEST_GUID_NO_READ_ACCESS "guid:e1424365-4ac5-43c2-a74b-bceff612408e"
#define TEST_GUID_NO_WRITE_ACCESS "guid:f4dbd41e-509c-4769-a86f-ca49bb0a9533"
#define TEST_GUID_NOEXIST_ACCESS "guid:ae595782-81be-40af-ac3c-8b1a084bfc82"

// srm part constant
#define TEST_SRM_VALID_SURL_EXAMPLE1 "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54"
#define TEST_SRM_INVALID_SURL_EXAMPLE2 "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/test2"
// access
#define TEST_SRM_ONLY_READ_ACCESS "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testread0011"
#define TEST_SRM_NOEXIST_ACCESS TEST_SRM_INVALID_SURL_EXAMPLE2
#define TEST_SRM_NO_READ_ACCESS "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testnoread0011"
#define TEST_SRM_WRITE_ACCESS "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testwrite0011"
#define TEST_SRM_NO_WRITE_ACCESS "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testnowrite0011"

#define TEST_SRM_TURL_EXAMPLE1 "gsiftp://atlas-storage-18.roma1.infn.it/atlas-storage-18.roma1.infn.it:/data4/dteam/2011-03-10/file75715ccc-1c54-4d18-8824-bdd3716a2b54.33321198.0"
#define TEST_SRM_IFCE_INVALID_PATH_REP "[SE][StatusOfPutRequest][SRM_INVALID_PATH] <none>"

// chmod 
#define TEST_LFC_MOD_READ_FILE "lfn:/grid/dteam/testchmodread0011"
#define TEST_LFC_MOD_UNEXIST_FILE "lfn:/grid/dteam/OpenSourceAndMicrosoft"
#define TEST_LFC_MOD_WRITE_FILE "lfn:/grid/dteam/testchmodwrite0011"

#define TEST_GUID_MODE_READ_FILE "guid:6eb27247-4387-4899-a37b-c4d40f1264fc"

#define TEST_LFC_RENAME_VALID_DEST "lfn:/grid/dteam/testrename0012"
#define TEST_LFC_RENAME_VALID_SRC "lfn:/grid/dteam/testrename0011"

#define TEST_LFC_MOVABLE_DIR_SRC "lfn:/grid/dteam/testmovedir0011/"
#define TEST_LFC_MOVABLE_DIR_DEST "lfn:/grid/dteam/testmovedir0012/"

// rename
#define TEST_GFAL_LOCAL_FILE_RENAME_SRC "/tmp/testrename0011"
#define TEST_GFAL_LOCAL_FILE_RENAME_DEST "/tmp/testrename0012"


// stat
#define TEST_GFAL_LFC_FILE_STAT_OK "lfn:/grid/dteam/teststat0011"
#define TEST_GFAL_LFC_LINK_STAT_OK  "lfn:/grid/dteam/teststatlink0011"
#define TEST_GFAL_LFC_FILE_STAT_NONEXIST TEST_LFC_NOEXIST_ACCESS


#define TEST_GFAL_SRM_FILE_STAT_OK "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/teststat0011"
#define TEST_GFAL_SRM_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_SRM_FILE_STAT_MODE_VALUE 0100664
#define TEST_GFAL_SRM_FILE_STAT_UID_VALUE 2
#define TEST_GFAL_SRM_FILE_STAT_GID_VALUE 2


#define TEST_GFAL_GUID_FILE_STAT_OK "guid:b7ba213b-829d-4a2e-b744-7c9cc685fe2d"

#define TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 0100555
#define TEST_GFAL_LFC_FILE_STAT_UID_VALUE 20124
#define TEST_GFAL_LFC_FILE_STAT_GID_VALUE 2688


#define TEST_GFAL_LFC_LINK_STAT_SIZE_VALUE 0
#define TEST_GFAL_LFC_LINK_STAT_MODE_VALUE 0120777
#define TEST_GFAL_LFC_LINK_STAT_UID_VALUE 20124
#define TEST_GFAL_LFC_LINK_STAT_GID_VALUE 2688

#define TEST_GFAL_LOCAL_STAT_COMMAND "dd if=/dev/zero of=/tmp/teststat0011 bs=512 count=4 &> /dev/null"
#define TEST_GFAL_LOCAL_LINK_COMMAND "ln -s /tmp/teststat0011 /tmp/teststatlink0011 &> /dev/null"
#define TEST_GFAL_LOCAL_STAT_OK "file:///tmp/teststat0011"
#define TEST_GFAL_LOCAL_LINK_OK "file:///tmp/teststatlink0011"
#define TEST_GFAL_LOCAL_STAT_NONEXIST "file:///tmp/ChristmasTree"

#define TEST_GFAL_LOCAL_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_LOCAL_FILE_STAT_MODE_VALUE 0100664


// mkdir

#define TEST_LFC_EEXIST_MKDIR "lfn:/grid/dteam/testmkdir0011"
#define TEST_LFC_BASE_FOLDER_URL_MKDIR1 "lfn:/grid/dteam/testmkdir0011/"
#define TEST_LFC_UNACCESS_MKDIR "lfn:/grid/dteam/testmkdirunaccess0011/testunacess0011"


#define TEST_LOCAL_MKDIR_EXIST_COMMAND "mkdir /tmp/testmkdir0011 &> /dev/null"
#define TEST_LOCAL_MKDIR_EXIST_FILE "file:///tmp/testmkdir0011"

#define TEST_LOCAL_BASE_FOLDER_URL_MKDIR1 "file:///tmp/"
#define TEST_LOCAL_UNACCESS_MKDIR "file:///root/foo"

#define TEST_SRM_EEXIST_MKDIR "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testmkdir0011"
#define TEST_SRM_BASE_FOLDER_URL_MKDIR1 "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/"
#define TEST_SRM_UNACCESS_MKDIR "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/testmkdir0012/testunaccessmkdir0011"
