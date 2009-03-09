
/*
 * Copyright (C) 2009 by CERN
 */

/*
 * @(#)$RCSfile: gfal_checksum.c,v $ $Revision: 1.1 $ $Date: 2009/03/09 15:16:38 $ CERN Remi Mollon
 */

#include "gfal_types.h"

static char *gfal_cksm_algname[] = {
	"Unknown",
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
