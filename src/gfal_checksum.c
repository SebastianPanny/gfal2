
/*
 * Copyright (C) 2009 by CERN
 */

/*
 * @(#)$RCSfile: gfal_checksum.c,v $ $Revision: 1.3 $ $Date: 2009/03/10 08:18:32 $ CERN Remi Mollon
 */

#include "gfal_types.h"

static char *gfal_cksm_algname[] = {
	"None",
	"CRC32",
	"ADLER32",
	"MD5",
	"SHA1",
	"SHA256",
	"SHA512"
};

const char *
gfal_get_checksum_algname (enum gfal_cksm_type cksmtype)
{
	return (gfal_cksm_algname[cksmtype]);
}

enum gfal_cksm_type
gfal_parse_checksum_algname (const char *cksmtype_name)
{
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[1]) == 0)
		return (GFAL_CKSM_CRC32);
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[2]) == 0)
		return (GFAL_CKSM_ADLER32);
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[3]) == 0)
		return (GFAL_CKSM_MD5);
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[4]) == 0)
		return (GFAL_CKSM_SHA1);
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[5]) == 0)
		return (GFAL_CKSM_SHA256);
	if (strcasecmp (cksmtype_name, gfal_cksm_algname[6]) == 0)
		return (GFAL_CKSM_SHA512);

	/* else it is not supported... */
	return (GFAL_CKSM_NONE);
}
