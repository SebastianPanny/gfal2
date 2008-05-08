/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_version.c,v $ $Revision: 1.2 $ $Date: 2008/05/08 13:16:36 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	printf ("GFAL-client-%s\n", gfal_version ());
	exit (0);
}
