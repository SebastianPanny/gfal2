
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

/*
 * @(#)$RCSfile: gfal_checksum.c,v $ $Revision: 1.8 $ $Date: 2009/08/26 15:58:00 $ CERN Remi Mollon
 */

#include <stdlib.h>
#include <string.h>
#include "gfal_types.h"

static char *gfal_cksm_algname[] = {
	"None",
	"CRC32",
	"ADLER32",
	"MD5",
	"SHA1",
	NULL
};

#define _CKSM_ALGNAME_MAX_LENGTH 32

const char *
gfal_get_checksum_algname (enum gfal_cksm_type cksmtype)
{
	return (gfal_cksm_algname[cksmtype]);
}

enum gfal_cksm_type
gfal_parse_checksum_algname (const char *cksmtype_name)
{
	int i;

	if (cksmtype_name) {
	    /* copy the original string, because we may have to transform it */
	    char str[_CKSM_ALGNAME_MAX_LENGTH + 1];
	    strncpy(str, cksmtype_name, _CKSM_ALGNAME_MAX_LENGTH);
	    str[_CKSM_ALGNAME_MAX_LENGTH] = 0;
	    /* Convert to upper case */
	    for (i = 0; str[i]; ++i)
	        str[i] = toupper(str[i]);
	    /* Check if it is an abbreviation, if so, replace it with the good one */
	    if (strcmp("AD", str) == 0)
	        strcpy(str, "ADLER32");
	    else if (strcmp("CR", str) == 0)
            strcpy(str, "CRC32");
        else if (strcmp("MD", str) == 0)
            strcpy(str, "MD5");
		/* Now, try to find the checksum algorithm */
	    for (i = 1; gfal_cksm_algname[i]; ++i) {
			if (strcmp (str, gfal_cksm_algname[i]) == 0)
				return (i);
		}
	}
	/* else it is not supported... */
	return (GFAL_CKSM_NONE);
}
