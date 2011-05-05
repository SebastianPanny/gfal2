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
  * 
  \file gfal_constant.h
  \brief list of all external constants of gfal
 */


/*
 * @(#)$RCSfile: gfal_constants.h,v $ $Revision: 1.8 $ $Date: 2009/03/09 15:16:38 $ CERN Jean-Philippe Baud
 */

#ifndef _GFAL_CONSTANTS_H
#define _GFAL_CONSTANTS_H

/* used to check wether there is enough space on the SE */
#define GFAL_NEWFILE_SIZE         1024

#define GFAL_OPEN_MAX             1024
#define GFAL_DEFAULT_CATALOG_TYPE "lfc"
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

/* GFAL error level for gfal_errmsg */
#define GFAL_ERRLEVEL_ERROR       0
#define GFAL_ERRLEVEL_WARN        1
#define GFAL_ERRLEVEL_INFO        2

#define GFAL_VERBOSE_QUIET       -1    /*!<  \def GFAL_VERBOSE_QUIET API-mode, no messages is printed */
#define GFAL_VERBOSE_NORMAL       0    /*!< \def GFAL_VERBOSE_NORMAL only errors are printed */
#define GFAL_VERBOSE_VERBOSE      1    /*!< \def GFAL_VERBOSE_VERBOSE a bit more verbose information is printed */
#define GFAL_VERBOSE_DEBUG        2    /*!< \def GFAL_VERBOSE_DEBUG  extra information is printed */

#define GFAL_CKSM_DEFAULT         GFAL_CKSM_ADLER32   // MD5 is the default algo

#define GFAL_LFC_PREFIX "lfn:"
#define GFAL_GUID_PREFIX "guid:"

#define GFAL_URL_MAX_LEN 2048

#endif
