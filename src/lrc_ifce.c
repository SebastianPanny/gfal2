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
 * @(#)$RCSfile: lrc_ifce.c,v $ $Revision: 1.25 $ $Date: 2008/12/03 12:38:47 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <string.h>
#define WITH_NOGLOBAL
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "lrcH.h"
#include "edg_local_replica_catalogSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "lrcC.c"
#include "lrcClient.c"
char *lrc_endpoint;
extern char *rmc_endpoint;

#include "gfal_internals.h"

	static int
lrc_init (struct soap *soap, char *errbuf, int errbufsz)
{
	int flags;

	soap_init (soap);
	soap->namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL && (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL) {
		if (!gfal_is_nobdii ()) {
			if (get_rls_endpoints (&lrc_endpoint, &rmc_endpoint, errbuf, errbufsz) != 0) {
				errno = EINVAL;
				return (-1);
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
					"You have to define 'RMC_ENDPOINT' and 'LRC_ENDPOINT' environment variables, when BDII calls are disabled");
			errno = EINVAL;
			return (-1);
		}
	}

#ifdef GFAL_SECURE
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
	}
#endif
	gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
			"RMC/LRC are obsolete! Please use LFC instead.");
	return (0);
}


char *
lrc_get_catalog_endpoint(char *errbuf, int errbufsz) {
	struct soap soap;
	if(lrc_init(&soap, errbuf, errbufsz) < 0)
		return (NULL);
	return lrc_endpoint;
}

	int
lrc_replica_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lrc__getPfnsResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__getPfns (&soap, lrc_endpoint, "",
					(char *) guid, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHGUID")) {
			sav_errno = 0;
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		// return 'false' if guid does not exist in LRC
		if(sav_errno == 0)
			return (0);

		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (out._getPfnsReturn->__size==0?0:1);
}

	char *
lrc_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__guidForPfnResponse out;
	char *p;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_lrc__guidForPfn (&soap, lrc_endpoint, "",
					(char *) pfn, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHPFN")) {
			sav_errno = ENOENT;
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (NULL);
	}
	p = strdup (out._guidForPfnReturn);
	soap_end (&soap);
	soap_done (&soap);
	return (p);
}

lrc_guid_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lrc__guidExistsResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__guidExists (&soap, lrc_endpoint, "",
				(char *) guid, &out))) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);	
		errno = ECOMM;
		return (-1);
	}
	ret = out._guidExistsReturn ? 1 : 0;
	soap_end (&soap);
	soap_done (&soap);
	return (ret);
}

lrc_register_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__addMappingResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__addMapping (&soap, lrc_endpoint, "",
				(char *) guid, (char *) pfn, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "PFNEXISTS"))
				sav_errno = EEXIST;
			else if (strstr (soap.fault->faultcode, "VALUETOOLONG"))
				sav_errno = ENAMETOOLONG;
			else {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
				sav_errno = ECOMM;
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

lrc_setfilesize (const char *pfn, GFAL_LONG64 filesize, char *errbuf, int errbufsz)
{
	struct lrc__setStringPfnAttributeResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	char tmpbuf[21];

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	sprintf (tmpbuf, GFAL_LONG64_FORMAT, filesize);
	if ((ret = soap_call_lrc__setStringPfnAttribute (&soap, lrc_endpoint,
				"", (char *) pfn, "size", tmpbuf, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHPFN")) {
			sav_errno = ENOENT;
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

	char **
lrc_surlsfromguid (const char *guid, char *errbuf, int errbufsz)
{
	int i;
	int j;
	struct lrc__getPfnsResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	char **surlarray;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_lrc__getPfns (&soap, lrc_endpoint, "",
				(char *) guid, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHGUID")) {
			sav_errno = ENOENT;
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (NULL);
	}
	if ((surlarray = calloc (out._getPfnsReturn->__size + 1, sizeof(char *))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (NULL);
	}
	for (i = 0; i < out._getPfnsReturn->__size; i++) {
		if ((surlarray[i] = strdup (out._getPfnsReturn->__ptr[i])) == NULL) {
			for (j = 0; j < i; j++)
				free (surlarray[j]);
			free (surlarray);
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (NULL);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (surlarray);
}

lrc_unregister_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__removeMappingResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__removeMapping (&soap, lrc_endpoint, "",
				(char *) guid, (char *) pfn, &out))) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

int
lrc_fillsurls (gfal_file gf)
{
	int size = 0;
	char **surls;
	char errmsg[GFAL_ERRMSG_LEN];
	struct soap soap;

	if (gf == NULL || gf->guid == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (lrc_init (&soap, errmsg, GFAL_ERRMSG_LEN) < 0) {
		gf->errmsg = strdup (errmsg);
		gf->errcode = errno;
		return (-1);
	}

	surls = lrc_surlsfromguid (gf->guid, errmsg, GFAL_ERRMSG_LEN);
	if (surls == NULL) {
		gf->errmsg = strdup (errmsg);
		gf->errcode = errno;
		return (-1);
	}

	if (surls[0] != NULL) {
		// Put SURLs in order of preference:
		//   1. SURLs from default SE
		//   2. SURLs from local domain
		//   3. Others
		char dname[GFAL_HOSTNAME_MAXLEN];
		int i, random_ind;
		int next_defaultse = 0, next_local = 0, next_others = 0;
		char  *surl, *surl_tmp1, *surl_tmp2, *p1, *p2, *p3;
		char *default_se;

		srand ((unsigned) time (NULL));
		*dname = '\0';
		getdomainnm (dname, sizeof(dname));

		// Calculate the size of tab 'surls'
		for (size = 0; surls[size]; ++size) ;

		gf->nbreplicas = size;
		gf->replicas = (gfal_replica *) calloc (size, sizeof (gfal_replica));
		if (gf->replicas == NULL) {
			gf->errcode = errno;
			return (-1);
		}

		/* and get the default SE, it there is one */
		default_se = get_default_se(errmsg, GFAL_ERRMSG_LEN);

		for (i = 0; i < size; i++) {
			if ((surl = surls[i]) == NULL ||
					(strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)))
				// skip entries not in the form srm: or sfn:
				continue;

			if ((p1 = strchr (surl + 6, '/')) == NULL) continue; // no host name
			if ((p2 = strchr (surl + 6, '.')) == NULL) continue; // no domain name
			*p1 = '\0';

			if ((p3 = strchr (surl + 6, ':')))
				// remove port number
				*p3 = '\0';

			if ((gf->replicas[i] = (gfal_replica) calloc (1, sizeof (struct _gfal_replica))) == NULL) {
				gf->errcode = errno;
				return (-1);
			}

			if(default_se != NULL && strcmp(surl + 6, default_se) == 0) {
				// SURL from default SE
				// Normally only 1 SURL from default SE, so no need of randomize place
				*p1 = '/';
				if (p3) *p3 = ':';

				surl_tmp1 = gf->replicas[next_defaultse]->surl;
				surl_tmp2 = gf->replicas[next_local]->surl;
				gf->replicas[next_defaultse++]->surl = strdup (surl);
				if (surl_tmp1 && gf->replicas[next_local])
					gf->replicas[next_local++]->surl = surl_tmp1;
				if (surl_tmp2 && gf->replicas[next_others])
					gf->replicas[next_others++]->surl = surl_tmp2;

				free (surl);
				continue;
			}

			if (strcmp (p2 + 1, dname) == 0) {
				// SURL from local domain
				// Put it at random place amongst local surls
				*p1 = '/';
				if (p3) *p3 = ':';
				random_ind = (rand() % (next_local - next_defaultse + 1)) + next_defaultse;

				surl_tmp1 = gf->replicas[random_ind]->surl;
				surl_tmp2 = gf->replicas[next_local]->surl;
				gf->replicas[random_ind]->surl = strdup (surl);
				if (surl_tmp1 && gf->replicas[next_local])
					gf->replicas[next_local++]->surl = surl_tmp1;
				if (surl_tmp2 && gf->replicas[next_others])
					gf->replicas[next_others++]->surl = surl_tmp2;

				free (surl);
				continue;
			}

			// Other SURL...
			// Put it at random place amongst other surls
			*p1 = '/';
			if (p3) *p3 = ':';
			random_ind = (rand() % (next_others - next_local + 1)) + next_local;

			surl_tmp1 = gf->replicas[random_ind]->surl;
			gf->replicas[random_ind]->surl = strdup (surl);
			if (surl_tmp1 && gf->replicas[next_others])
				gf->replicas[next_others++]->surl = surl_tmp1;

			free (surl);
		}
	}

	if (surls) free (surls);
	errno = 0;
	return (0);
}
