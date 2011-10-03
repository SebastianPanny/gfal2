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
 
/**
  * @file gfal_constants.h 
  * the global declarations of gfal core
  * @author Devresse Adrien , Baud Jean-Philippe 
*/


#ifndef _GFAL_CONSTANTS_H
#define _GFAL_CONSTANTS_H

/* used to check wether there is enough space on the SE */
#define GFAL_NEWFILE_SIZE         1024
#define GFAL_MODULE_NAME_SIZE 	1024
#define GFAL_OPEN_MAX             1024
#define GFAL_DEFAULT_PLUGIN_TYPE "lfc"
#define GFAL_FQAN_MAX             64
#define GFAL_GUID_LEN             37
#define GFAL_VO_MAXLEN            255
#define GFAL_FQAN_MAXLEN          511
#define GFAL_HOSTNAME_MAXLEN      255
#define GFAL_PATH_MAXLEN          1024
#define GFAL_ERRMSG_LEN           1024
#define GFAL_BDII_TIMEOUT         60
#define GFAL_SIZE_MARGIN          1048576     // 1MB
#define GFAL_SRM_MAXRETRIES       10
#define GFAL_MAX_PLUGIN_ID_SIZE 512

/* GFAL error level for gfal_errmsg */
#define GFAL_ERRMSG_LEN			  1024
#define GFAL_ERRLEVEL_ERROR       0
#define GFAL_ERRLEVEL_WARN        1
#define GFAL_ERRLEVEL_INFO        2

#define GFAL_VERBOSE_NORMAL     0x00    /*!< \def GFAL_VERBOSE_NORMAL only errors are printed */
#define GFAL_VERBOSE_VERBOSE    0x01    /*!< \def GFAL_VERBOSE_VERBOSE a bit more verbose information is printed */
#define GFAL_VERBOSE_DEBUG      0x02    /*!< \def GFAL_VERBOSE_DEBUG  extra information is printed */
#define GFAL_VERBOSE_TRACE		0x08	/*! < \def GFAL_VERBOSE_TRACE execution trace */


#define GFAL_GUID_PREFIX "guid:"

// default buffer size for address
#define GFAL_URL_MAX_LEN 2048

#define GFAL_MODULEID_LOCAL "local_file_module"
#define GFAL_MODULEID_SRM 2
#define GFAL_EXTERNAL_MODULE_OFFSET 10

#define GFAL_PLUGIN_ENVAR "GFAL_PLUGIN_LIST"
#define GFAL_PLUGIN_INIT_SYM "gfal_plugin_init"
#define GFAL_MAX_PLUGIN_LIST 2048


// xattr std keys
#define GFAL_XATTR_REPLICA "user.replicas" // global key of replicas for the extended attributes 
#define GFAL_XATTR_GUID "user.guid" // global key for the guid of a file 
#define GFAL_XATTR_COMMENT "user.comment" // global key for the comments of a file 
#define GFAL_XATTR_CHKSUM_TYPE "user.chksumtype"
#define GFAL_XATTR_CHKSUM_VALUE "user.checksum"

// parameters attritutes
#define GFAL_PARAM_SET 0
#define GFAL_PARAM_GET 1

#define GFAL_PARAM_TRUE "True" // string for boolean parameters
#define GFAL_PARAM_FALSE "False"


// parameters list for core
#define GFAL_NO_BDII_OPTION "no_bdii" // set bdii value, can be True or False

#endif
