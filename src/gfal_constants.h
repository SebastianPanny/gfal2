/*
 * Copyright (C) 2003 by CERN
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

#define GFAL_VERBOSE_QUIET       -1    // API-mode, no messages is printed
#define GFAL_VERBOSE_NORMAL       0    // only errors are printed
#define GFAL_VERBOSE_VERBOSE      1    // a bit more verbose information is printed
#define GFAL_VERBOSE_DEBUG        2    // extra information is printed

#define GFAL_CKSM_DEFAULT         GFAL_CKSM_ADLER32   // MD5 is the default algo

#endif
