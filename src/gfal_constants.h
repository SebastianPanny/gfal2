/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_constants.h,v $ $Revision: 1.5 $ $Date: 2008/10/16 12:10:11 $ CERN Jean-Philippe Baud
 */

#ifndef _GFAL_CONSTANTS_H
#define _GFAL_CONSTANTS_H

#define GFAL_OPEN_MAX             1024
#define GFAL_DEFAULT_CATALOG_TYPE "lfc"
#define GFAL_FQAN_MAX             64
#define GFAL_GUID_LEN             37
#define GFAL_VO_MAXLEN            255
#define GFAL_FQAN_MAXLEN          511
#define GFAL_HOSTNAME_MAXLEN      255
#define GFAL_ERRMSG_LEN           1024
#define GFAL_BDII_TIMEOUT         60
#define GFAL_SIZE_MARGIN          1048576     // 1MB

/* GFAL error level for gfal_errmsg */
#define GFAL_ERRLEVEL_ERROR       0
#define GFAL_ERRLEVEL_WARN        1
#define GFAL_ERRLEVEL_INFO        2

#endif
