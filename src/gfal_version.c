/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_version.c,v $ $Revision: 1.1 $ $Date: 2007/10/10 14:26:44 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	printf ("GFAL-client-%s\n", gfal_version ());
	exit (0);
}
