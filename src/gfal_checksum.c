
/*
 * Copyright (C) 2009 by CERN
 */

/*
 * @(#)$RCSfile: gfal_checksum.c,v $ $Revision: 1.6 $ $Date: 2009/04/08 14:21:34 $ CERN Remi Mollon
 */

#include <stdlib.h>
#include <strings.h>
#include "gfal_types.h"

static char *gfal_cksm_algname[] = {
	"None",
	"CRC32",
	"ADLER32",
	"MD5",
	"SHA1",
	NULL
};


const char *
gfal_get_checksum_algname (enum gfal_cksm_type cksmtype)
{
	return (gfal_cksm_algname[cksmtype]);
}

enum gfal_cksm_type
gfal_parse_checksum_algname (const char *cksmtype_name)
{
	int i;

	if (cksmtype_name)
		for (i = 1; gfal_cksm_algname[i]; ++i) {
			if (strcasecmp (cksmtype_name, gfal_cksm_algname[i]) == 0)
				return (i);
		}

	/* else it is not supported... */
	return (GFAL_CKSM_NONE);
}
