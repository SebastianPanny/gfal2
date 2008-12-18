/*
 * Copyright (C) 2008-2010 by CERN
 */

/*
 * @(#)$RCSfile: gfal_timeouts.c,v $ $Revision: 1.4 $ $Date: 2008/12/18 13:09:17 $ CERN Remi Mollon
 */

#include <stdlib.h>
#include <string.h>
#include "gfal_internals.h"

static int gfal_timeout_connect = 60;
static int gfal_timeout_sendreceive = 0;
static int gfal_timeout_bdii = 60;
static int gfal_timeout_srm = 3600;

void
gfal_set_timeout_connect (int value) {
	char *lfc_timeout = NULL;

	if (value >= 0)
		gfal_timeout_connect = value;

	/* if 'LFC_CONNTIMEOUT is defined, it isn't overwritten */
	if ((lfc_timeout = getenv ("LFC_CONNTIMEOUT")) == NULL) {
		asprintf (&lfc_timeout, "LFC_CONNTIMEOUT=%d", value);
		if (lfc_timeout == NULL)
			return;
		if (putenv (lfc_timeout) < 0)
			return;
	} else
		free (lfc_timeout);
}

int gfal_get_timeout_connect () {
	return (gfal_timeout_connect);
}

void
gfal_set_timeout_sendreceive (int value) {
	if (value >= 0)
		gfal_timeout_sendreceive = value;
}

int gfal_get_timeout_sendreceive () {
	return (gfal_timeout_sendreceive);
}

void
gfal_set_timeout_bdii (int value) {
	if (value >= 0)
		gfal_timeout_bdii = value;
}

int gfal_get_timeout_bdii () {
	return (gfal_timeout_bdii);
}

void
gfal_set_timeout_srm (int value) {
	if (value >= 0)
		gfal_timeout_srm = value;
}

int gfal_get_timeout_srm () {
	return (gfal_timeout_srm);
}
