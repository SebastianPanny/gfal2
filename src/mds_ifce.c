/*                                                                -*- mode: c; c-basic-offset: 8;  -*-
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.68 $ $Date: 2008/07/11 10:24:57 $ CERN Jean-Philippe Baud
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
#include "gfal_api.h"

#define GFAL_VOINFOTAG_DEFAULT "DEFAULT"

/* bug #38585: GFAL: querying BDII using the 'resource' branch (as well)
 * Need to get both 'local' and 'resource' entries
 * static char *dn = "mds-vo-name=local,o=grid";
 */
static char *dn = "o=grid";
static char *dn_filter = "(|(mds-vo-name:dn:=local)(mds-vo-name:dn:=resource))";
static const char gfal_remote_type[] = "BDII";

struct bdii_server_info_t {
	const char* server;
	int port;
};

static struct bdii_server_info_t *bdii_servers = NULL;
static int bdii_servers_count = 0;
static int bdii_timeout = DEFAULT_BDII_TIMEOUT;
static int bdii_server_current = 0;
static int bdii_server_known_good = 0;

/*
   Find the number of occurences of char c in string str.
   */
static int
strchrscan (const char* str, int c)
{
	int n = 0;
	while (*str != 0)
		if (* (str++) == c) ++n;

	return n;
}	     

/* Convert LDAP error into errno */
static int
ldaperr2errno (int err) {
	int res;

	switch (err) {
		case LDAP_SERVER_DOWN:
			res = EHOSTDOWN; break;
		default:
			res = ECONNREFUSED;
	}

	return res;
}

/*
   Parse the LCG_GFAL_INFOSYS environment variable and fill out the
   bdii_servers_count and bdii_servers vars. 
   Usually this function is called only once per program startup.
   */
static int
bdii_parse_env (char *errbuf, int errbufsz)
{
	static const char *separator = ",";
	char *bdii_env;
	int max_servers_count;
	struct bdii_server_info_t *list;
	int n;
	char *ptr, *colon_pos;
	char *strtok_state;
	char errmsg[ERRMSG_LEN];

	bdii_env = getenv ("LCG_GFAL_BDII_TIMEOUT");
	if (bdii_env != NULL) {
		bdii_timeout = atoi (bdii_env);
		if (bdii_timeout <= 0) {
			bdii_servers_count = -1;
			snprintf (errmsg, ERRMSG_LEN, "LCG_GFAL_BDII_TIMEOUT: invalid value (%d)", bdii_timeout);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			return (-1);
		}
	}

	bdii_env = getenv ("LCG_GFAL_INFOSYS");
	if (bdii_env == NULL) {
		bdii_servers_count = -1;
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_INFOSYS not set");
		errno = EINVAL;
		return (-1);
	}

	max_servers_count = strchrscan (bdii_env, separator[0]) + 1;
	list = (struct bdii_server_info_t*) calloc (max_servers_count, sizeof (struct bdii_server_info_t));
	if (list == NULL) {
		bdii_servers_count = -1;
		return (-1);
	}

	ptr = strtok_r (bdii_env, separator, &strtok_state);

	for (n = 0; ptr != NULL; ++n) {
		if (!isalnum (*ptr)) { /* ignore this invalid token */
			ptr = strtok_r (NULL, separator, &strtok_state);
			continue;
		}

		colon_pos = strchr (ptr, ':');
		if (colon_pos != NULL) {
			list[n].port = atoi (colon_pos + 1);
			*colon_pos = 0;
		} else { /* port not specified, defaulting to 2170 */
			list[n].port = 2170;
		}

		list[n].server = strdup (ptr);
		ptr = strtok_r (NULL, separator, &strtok_state);
	}

	if (n == 0) {
		free (list);
		bdii_servers_count = -1;
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_INFOSYS is invalid");
		errno = EINVAL;
		return (-1);
	}

	bdii_servers = realloc (list, n * sizeof (struct bdii_server_info_t));
	if (bdii_servers == NULL) {
		/* errno is set by realloc */
		bdii_servers_count = -1;
		return -1;
	}

	bdii_servers_count = n;

	GFAL_DEBUG ("DEBUG: BDII environment parsing results:\n"
			"DEBUG: BDII base timeout: %d\n"
			"DEBUG: BDII servers list:\n", bdii_timeout);

	for (n = 0; n < bdii_servers_count; ++n)
		GFAL_DEBUG ("DEBUG:  - %s:%d\n", bdii_servers[n].server, bdii_servers[n].port);

	return bdii_servers_count;
}

/*
   Try to find a next bdii server.
   Return 1 if there are still some servers to try or 0 if tried all servers.
   */
static int
bdii_server_get_next (const char** bdii_server_ptr, int *bdii_port_ptr)
{
	bdii_server_current = (bdii_server_current + 1) % bdii_servers_count;

	if (bdii_server_current == bdii_server_known_good) {
		return 0;
	}

	*bdii_server_ptr = bdii_servers[bdii_server_current].server;
	*bdii_port_ptr = bdii_servers[bdii_server_current].port;
	return 1;
}

/*
   Get the current bdii server address
   */
static void
bdii_server_get_current (const char** bdii_server_ptr, int *bdii_port_ptr)
{
	*bdii_server_ptr = bdii_servers[bdii_server_current].server;
	*bdii_port_ptr = bdii_servers[bdii_server_current].port;
}

/*
   Mark the current bdii server as a good one.
   */
static void
bdii_server_is_good (void)
{
	bdii_server_known_good = bdii_server_current;
}

/*
   make a query to the BDII.
   Store the LDAP* into the ld_ptr, LDAPMessage* into reply_ptr, 
   and BDII server used into bdii_server_ptr and bdii_port_ptr.
   */
static int
bdii_query_send (LDAP** ld_ptr, char* filter, char* attrs[],
		LDAPMessage **reply_ptr, const char** bdii_server_ptr, int *bdii_port_ptr,
		char *errbuf, int errbufsz)
{
	const char *bdii_server;
	int bdii_port;
	LDAP *ld;
	char *complete_filter = NULL;
	char errmsg[ERRMSG_LEN];
	struct timeval timeout;
	int err = 0, rc = 0;

	if (gfal_is_nobdii ()) {
		gfal_errmsg (errbuf, errbufsz, "BDII calls are needed, but disabled!");
		errno = EINVAL;
		return (-1);
	}

	/* Parse the environment, if required. */
	if (bdii_servers_count == 0) {
		if (bdii_parse_env (errbuf, errbufsz) < 0)
			return (-1);
	}
	if (bdii_servers_count < 0) {
		gfal_errmsg (errbuf, errbufsz, "Invalid BDII parameters");
		errno = EINVAL;
		return (-1);
	}

	/* Add the dn filter part to 'filter' */
	if (asprintf (&complete_filter, "(& %s %s)", dn_filter, filter) < 0 || complete_filter == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	bdii_server_get_current (&bdii_server, &bdii_port);
	do {
		ld = ldap_init (bdii_server, bdii_port);
		*bdii_server_ptr = bdii_server;
		*bdii_port_ptr = bdii_port;
		if (ld == NULL) continue;

		timeout.tv_sec = bdii_timeout;
		timeout.tv_usec = 0;

		ldap_set_option (ld, LDAP_OPT_NETWORK_TIMEOUT, &timeout);
		if ((err = ldap_simple_bind_s (ld, "", "")) != LDAP_SUCCESS) {
			ldap_unbind (ld);
			snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: %s", gfal_remote_type, bdii_server, bdii_port, ldap_err2string (err));
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ldaperr2errno (err);
			GFAL_DEBUG ("DEBUG: %s\n", errmsg);
			continue;
		}

		rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, complete_filter, attrs, 0, &timeout, reply_ptr);
		if (rc != LDAP_SUCCESS) {
			ldap_unbind (ld);
			if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
				snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: Connection Timeout", gfal_remote_type, bdii_server, bdii_port);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				errno = ETIMEDOUT;
			} else {
				snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: ERROR: %s", gfal_remote_type, bdii_server, bdii_port, 
						ldap_err2string (rc));
				gfal_errmsg (errbuf, errbufsz, errmsg);
				errno = EINVAL;
			}
			GFAL_DEBUG ("DEBUG: %s\n", errmsg);
			continue;
		}

		*ld_ptr = ld;
		bdii_server_is_good ();
		return 0;
	} while (bdii_server_get_next (&bdii_server, &bdii_port));

	return -1;
}

static int
bdii_query_free (LDAP** ld_ptr, LDAPMessage **reply_ptr)
{
	ldap_msgfree (*reply_ptr);
	ldap_unbind (*ld_ptr);
	return 0;
}

/* get BDII hostname and port number */
/* backwards compatibility version */
get_bdii (char *bdii_server, int buflen, int *bdii_port, char *errbuf, int errbufsz)
{
	const char *bdii_server_r;
	int bdii_port_r;

	/* Parse the environment, if required. */
	if (bdii_servers_count == 0) {
		if (bdii_parse_env (errbuf, errbufsz) < 0)
			return (-1);
	}
	if (bdii_servers_count < 0) {
		gfal_errmsg (errbuf, errbufsz, "Invalid BDII parameters");
		errno = EINVAL;
		return (-1);
	}

	bdii_server_get_current (&bdii_server_r, &bdii_port_r);
	if (strlen (bdii_server_r) >= buflen) {
		gfal_errmsg (errbuf, errbufsz, "bdii_server buffer length is too short");
		errno = EINVAL;
		return -1;
	}

	strcpy (bdii_server, bdii_server_r);
	*bdii_port = bdii_port_r;

	return (0);
}

/* generates part of bdii filter with accesscontrolbaserule */
static char *
generate_acbr (const char *glueobject, char *errbuf, int errbufsz) {
	char tmp[64 + FQAN_MAXLEN];
	char *filter = NULL, *vo = NULL, **fqan = NULL;
	int nb_fqan, filterlen, i;

	if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL)
		return (NULL);

	nb_fqan = gfal_get_fqan (&fqan, errbuf, errbufsz);
	filterlen = nb_fqan * (sizeof (tmp)) + 3 * (64 + VO_MAXLEN);
	
	if ((filter = (char *) calloc (filterlen, sizeof (char))) == NULL)
		return (NULL);

	snprintf (filter, filterlen,
			"(| (%sAccessControlBaseRule=VO:%s) (%sAccessControlBaseRule=%s) (%sAccessControlRule=%s)",
			glueobject, vo, glueobject, vo, glueobject, vo);

	for (i = 0; i < nb_fqan; ++i) {
		snprintf (tmp, sizeof (tmp), " (%sAccessControlBaseRule=VOMS:%s)", glueobject, fqan[i]);
		strcat (filter, tmp);
	}

	strcat (filter, ")");
	return (filter);
}

/* get from the BDII the CE Accesspoint for a given SE */

get_ce_ap (const char *host, char **ce_ap, char *errbuf, int errbufsz)
{
	static char ce_ap_atnm[] = "GlueCESEBindCEAccesspoint";
	static char *template = " (GlueCESEBindSEUniqueID=%s)";
	static char *attrs[] = {ce_ap_atnm, NULL};
	int bdii_port;
	const char *bdii_server;
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (strlen (template) + strlen (host) - 2 >= sizeof (filter)) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: Hostname too long", gfal_remote_type, bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, host);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	if (rc < 0) return rc;
	GFAL_DEBUG ("DEBUG: get_ce_ap used server %s:%d\n", bdii_server, bdii_port);

	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, ce_ap_atnm);
		if (value == NULL) {
			snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: CE Accesspoint not found for host %s", gfal_remote_type, bdii_server, bdii_port, host);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;

		} else {
			if ( (*ce_ap = strdup (value[0])) == NULL)
				rc = -1;
			ldap_value_free (value);
		}
	} else {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: No GlueCESEBind found for host %s", gfal_remote_type, bdii_server, bdii_port, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	bdii_query_free (&ld, &reply);
	return (rc);
}

/* get from the BDII the RLS endpoints */

get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint, char *errbuf, int errbufsz)
{
	static char rls_ep[] = "GlueServiceEndpoint";
	static char rls_type[] = "GlueServiceType";
	static char *template = " (& (GlueServiceType=*) (| (GlueServiceAccessControlBaseRule=%s) (GlueServiceAccessControlRule=%s)))";
	char *attr;
	static char *attrs[] = {rls_type, rls_ep, NULL};
	int bdii_port;
	const char *bdii_server;
	BerElement *ber;
	LDAPMessage *entry;
	char filter[100 + 2 * VO_MAXLEN];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char *service_type;
	char *service_url;
	char **value;
	char *vo;
	char errmsg[ERRMSG_LEN];

	if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if (strlen (vo) > VO_MAXLEN) {
		gfal_errmsg (errbuf, errbufsz, "VO name too long");
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, vo, vo);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	if (rc < 0) return rc;
	GFAL_DEBUG ("DEBUG: get_rls_endpoints used server %s:%d\n", bdii_server, bdii_port);

	for (entry = ldap_first_entry (ld, reply);
			entry != NULL;
			entry = ldap_next_entry (ld, entry)) {
		service_type = NULL;
		service_url = NULL;
		for (attr = ldap_first_attribute (ld, entry, &ber);
				attr != NULL;
				attr = ldap_next_attribute (ld, entry, ber)) {
			value = ldap_get_values (ld, entry, attr);
			if (value != NULL) {
				if (strcmp (attr, "GlueServiceType") == 0) {
					if ((service_type = strdup (value[0])) == NULL)
						rc = -1;
				} else {	/* GlueServiceEndpoint */
					if ((service_url = strdup (value[0])) == NULL)
						rc = -1;
				}
				ldap_value_free (value);
			}
			else 
				rc = -1;
		}
		if (rc == 0) {
			if (strcmp (service_type, "edg-replica-metadata-catalog") == 0) {
				if ( (*rmc_endpoint = strdup (service_url)) == NULL)
					rc = -1;
			} else if (strcmp (service_type, "edg-local-replica-catalog") == 0) {
				if ( (*lrc_endpoint = strdup (service_url)) == NULL)
					rc = -1;
			}
		}
		free (service_type);
		free (service_url);
	}
	if (*lrc_endpoint == NULL) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: LRC endpoint not found", gfal_remote_type, bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	if (*rmc_endpoint == NULL) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: RMC endpoint not found", gfal_remote_type, bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	bdii_query_free (&ld, &reply);
	return (rc);
}

/* get from the BDII the endpoint for the LFC */

get_lfc_endpoint (char **lfc_endpoint, char *errbuf, int errbufsz)
{
	static char ep[] = "GlueServiceEndpoint";
	static char *template = " (& (GlueServiceType=lcg-file-catalog) %s)";
	static char *attrs[] = {ep, NULL};
	LDAPMessage *entry;
	char *filter, *filter_tmp;
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char **value;
	char errmsg[ERRMSG_LEN];
	const char *bdii_server;
	int bdii_port;

	*lfc_endpoint = NULL;
	
	if ((filter_tmp = generate_acbr ("GlueService", errbuf, errbufsz)) == NULL)
		return (-1);

	rc = asprintf (&filter, template, filter_tmp);
	free (filter_tmp);
	if (rc < 0) return (-1);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	free (filter);
	if (rc < 0) return -1;
	GFAL_DEBUG ("DEBUG: get_lfc_endpoint used server %s:%d\n", bdii_server, bdii_port);

	for (entry = ldap_first_entry (ld, reply);
			entry != NULL;
			entry = ldap_next_entry (ld, entry)) {
		if ( (value = ldap_get_values (ld, entry, ep)) == NULL || *value == NULL)
			continue;
		if ( (*lfc_endpoint = strdup (*value)) == NULL) {
			errno = ENOMEM;
			rc = -1;
		}
		ldap_value_free (value);
		break;
	}

	bdii_query_free (&ld, &reply);

	if (rc == 0 && *lfc_endpoint == NULL) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: LFC endpoint not found", gfal_remote_type, bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}

	return (rc);
}

/* Get from the BDII the SAPath */

static int
get_sa_path (const char *host, const char *salocalid, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
	static char sa_path_atnm[] = "GlueSAPath";
	static char sa_root_atnm[] = "GlueSARoot";
	static char *template =
		"(& (GlueSALocalID=%s) (GlueChunkKey=GlueSEUniqueID=%s))";
	static char *attrs[] = {sa_root_atnm, sa_path_atnm, NULL};
	int bdii_port;
	const char *bdii_server;
	LDAPMessage *entry;
	char *filter;
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char **value;
	char *vo;
	char errmsg[ERRMSG_LEN];

	if (!host || !sa_path) {
		gfal_errmsg (errbuf, errbufsz, "get_sa_path: invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	if (strlen (host) > HOSTNAME_MAXLEN) {
		snprintf (errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}

	if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL)
		return (-1);

	if (salocalid == NULL) {
		salocalid = vo;
	}

	rc = asprintf (&filter, template, salocalid, host);
	if (rc < 0) return (-1);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	free (filter);
	if (rc < 0) return rc;
	GFAL_DEBUG ("DEBUG: get_sa_path used server %s:%d\n", bdii_server, bdii_port);

	*sa_path = *sa_root = NULL;
	entry = ldap_first_entry (ld, reply);

	if (entry) {
		if ((value = ldap_get_values (ld, entry, sa_path_atnm)) != NULL) {
			/* We deal with pre-LCG 2.7.0 where SA Path was incorrect and had vo: prefix */
			if ((strncmp (value[0], vo, strlen (vo)) == 0) && (* (value[0] + strlen (vo)) == ':')) {
				if ( (*sa_path = strdup (value[0] + strlen (vo) + 1)) == NULL)
					rc = -1;
			} else if ( (*sa_path = strdup (value[0])) == NULL)
					rc = -1;
			ldap_value_free (value);
		}
		else if ((value = ldap_get_values (ld, entry, sa_root_atnm)) != NULL) {
			if ((*sa_root = strdup (value[0] + strlen (vo) + 1)) == NULL)
				rc = -1;
			ldap_value_free (value);
		} else {
			snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: Both SAPath and SARoot are not set about %s VO and SE : %s",
					gfal_remote_type, bdii_server, bdii_port, vo, host);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;
		}

	} else {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: No GlueSA information found about %s VO and SE %s",
				gfal_remote_type, bdii_server, bdii_port, vo, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;               
	}

	bdii_query_free (&ld, &reply);
	return (rc);
}

static int
get_voinfo (const char *host, const char *spacetokendesc, char **sa_path, char **salocalid, char *errbuf, int errbufsz)
{
	static char sa_path_atnm[] = "GlueVOInfoPath";
	static char sa_key_atnm[] = "GlueChunkKey";
	static char *template =
		"(& %s (GlueVOInfoTag=%s) (GlueChunkKey=GlueSEUniqueID=%s))";
	static char *attrs[] = {sa_key_atnm, sa_path_atnm, NULL};
	int i;
	int bdii_port;
	const char *bdii_server;
	LDAPMessage *entry;
	char *filter, *filter_tmp;
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (!host || !sa_path || !salocalid) {
		gfal_errmsg (errbuf, errbufsz, "get_voinfo: invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	if (strlen (host) > HOSTNAME_MAXLEN) {
		snprintf (errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	
	if ((filter_tmp = generate_acbr ("GlueVOInfo", errbuf, errbufsz)) == NULL)
		return (-1);

	rc = asprintf (&filter, template, filter_tmp, spacetokendesc ? spacetokendesc : GFAL_VOINFOTAG_DEFAULT, host);
	free (filter_tmp);
	if (rc < 0) return (-1);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	free (filter);
	if (rc < 0) return rc;
	GFAL_DEBUG ("DEBUG: get_voinfo used server %s:%d\n", bdii_server, bdii_port);

	*sa_path = *salocalid = NULL;
	entry = ldap_first_entry (ld, reply);

	if (entry) {
		if ((value = ldap_get_values (ld, entry, sa_path_atnm)) != NULL) {
			if ( (*sa_path = strdup (value[0])) == NULL)
				rc = -1;
			ldap_value_free (value);
		}
		else if ((value = ldap_get_values (ld, entry, sa_key_atnm)) != NULL) {
			rc = 0;
			for (i = 0; value[i] && !*salocalid && !rc; ++i) {
				if (strncmp (value[i], "GlueSALocalID=", 14) == 0) {
					if ( (*salocalid = strdup (value[i] + 14)) == NULL)
						rc = -1;
				}
			}
			ldap_value_free (value);
		} else {
			snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: Warning, GlueVOInfo for tag '%s' and SE '%s' wrongly published",
					gfal_remote_type, bdii_server, bdii_port, spacetokendesc, host);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			rc = -1;
		}

	} else {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s:%d: Warning, no GlueVOInfo information found about tag '%s' and SE '%s'",
				gfal_remote_type, bdii_server, bdii_port, spacetokendesc, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		rc = -1;               
	}

	bdii_query_free (&ld, &reply);
	return (rc);
}

get_storage_path (const char *host, const char *spacetokendesc, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
	char *salocalid = NULL;
	int rc = 0;

	if (!host || !sa_path || !sa_root) {
		gfal_errmsg (errbuf, errbufsz, "get_storage_path: invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	*sa_path = *sa_root = NULL;

	if (spacetokendesc)
		rc = get_voinfo (host, spacetokendesc, sa_path, &salocalid, errbuf, errbufsz);

	if (!*sa_path)
		rc = get_sa_path (host, salocalid, sa_path, sa_root, errbuf, errbufsz);

	return (rc);
}


/* get from the BDII the SE type (disk, srm_v1) */

get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
	static char se_type_atpt[] = "GlueSEPort";
	static char se_type_atst[] = "GlueSEStatus";
	static char se_type_atve[] = "GlueServiceVersion";
	static char se_type_atty[] = "GlueServiceType";
	static char se_type_atep[] = "GlueServiceEndpoint";
	static char *template = " (| (GlueSEUniqueID=%s) (& (GlueServiceType=srm*) (GlueServiceEndpoint=*://%s*)))";
	static char *attrs[] = {se_type_atpt, se_type_atst, se_type_atve, se_type_atty, se_type_atep, NULL};
	char host_tmp[HOSTNAME_MAXLEN];
	int len_tmp;
	char *port;
	int bdii_port;
	const char *bdii_server;
	LDAPMessage *entry;
	char filter[2 * HOSTNAME_MAXLEN + 110];
	int i, nbentries, n, rc = 0;
	LDAP *ld;
	LDAPMessage *reply;
	char **sep = NULL, **stp = NULL, **st = NULL, **sv = NULL, **ep = NULL, **value = NULL;
	char errmsg[ERRMSG_LEN];

	*se_types = NULL;
	*se_endpoints = NULL;
	len_tmp = strlen (host);

	if (len_tmp >= HOSTNAME_MAXLEN) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s: Hostname too long", gfal_remote_type, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}

	strncpy (host_tmp, host, HOSTNAME_MAXLEN);
	if ((port = strchr (host_tmp, ':')) != NULL)
		*port = 0;

	sprintf (filter, template, host_tmp, host);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	if (rc < 0) return (-1);
	GFAL_DEBUG ("DEBUG: get_se_types_and_endpoints used server %s:%d\n", bdii_server, bdii_port);

	rc = 0;
	
	if ((nbentries = ldap_count_entries (ld, reply)) < 1) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s: No entries for host: %s", gfal_remote_type, bdii_server, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		bdii_query_free (&ld, &reply);
		return (-1);
	}

	nbentries++;
	if ( (st = calloc (nbentries, sizeof (char *))) == NULL ||
			(sv = calloc (nbentries, sizeof (char *))) == NULL ||
			(ep = calloc (nbentries, sizeof (char *))) == NULL ||
			(stp = calloc (nbentries, sizeof (char *))) == NULL ||
			(sep = calloc (nbentries, sizeof (char *))) == NULL) {
		errno = ENOMEM;
		if (st) free (st);
		if (sv) free (sv);
		if (ep) free (ep);
		if (stp) free (stp);
		bdii_query_free (&ld, &reply);
		return (-1);
	}

	for (entry = ldap_first_entry (ld, reply), n = 0;
			entry != NULL && rc == 0;
			entry = ldap_next_entry (ld, entry)) {

		if ((value = ldap_get_values (ld, entry, se_type_atep)) != NULL) {
			// GlueService entry

			ep[n] = strdup (value[0]);
			ldap_value_free (value);

			if ((value = ldap_get_values (ld, entry, se_type_atty)) == NULL) {
				ldap_value_free (value);
				free (ep[n]);
				continue;
			}
			st[n] = strdup (value[0]);
			ldap_value_free (value);

			if ((value = ldap_get_values (ld, entry, se_type_atve)) == NULL) {
				ldap_value_free (value);
				free (ep[n]);
				free (st[n]);
				continue;
			}
			sv[n] = strdup (value[0]);
			ldap_value_free (value);
			++n;

		} else {
			// GlueSE entry
			// NB: there is only one GlueSE entry per SE!
			// (even if there are several interfaces, like srm_v1 and srm_v2)

			/* Due to some unofficial values, it has been disabled !!
			 *
			if ((value = ldap_get_values (ld, entry, se_type_atst)) != NULL &&
					strcasecmp (value[0], "production") != 0) {
				snprintf (errmsg, ERRMSG_LEN, "[%s] %s: is not in 'production' status in BDII ('%s')", gfal_remote_type, host, value[0]);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				ldap_value_free (value);
				errno = EINVAL;
				rc = -1;
				break;
			}
			else ; 
			*/

			if (port == NULL) {
				// If port is not yet defined in host_tmp, and is available
				// it is copied to host_tmp buffer
				// ... But it will only be used if there is no GlueService entry
				ldap_value_free (value);
				port = host_tmp + len_tmp;
				value = ldap_get_values (ld, entry, se_type_atpt);
				if (value == NULL) {
					continue;
				} else if (len_tmp + strlen (value[0]) + 1 < HOSTNAME_MAXLEN) {
					strcpy (port + 1, value[0]);
				} else {
					snprintf (errmsg, ERRMSG_LEN, "[%s] %s: Hostname too long", gfal_remote_type, host);
					gfal_errmsg (errbuf, errbufsz, errmsg);
					ldap_value_free (value);
					errno = ENAMETOOLONG;
					rc = -1;
					break;
				}
			}

			ldap_value_free (value);
		}
	}
	bdii_query_free (&ld, &reply);

	if (rc < 0) {
		for (i = 0; i < n; i++) {
			if (st[i]) free (st[i]);
			if (sv[i]) free (sv[i]);
			if (ep[i]) free (ep[i]);
		}
		free (st);
		free (sv);
		free (ep);
		free (stp);
		free (sep);
	} else {
		if (n > 0) {
			// If there are GlueServices entries...
			for (i = 0; i < n; ++i) {
				if ((strcmp (st[i], "srm_v1") == 0 || strcmp (st[i], "srm_v2") == 0) && ep[i]) {
					stp[i] = strdup (st[i]);
					sep[i] = strdup (ep[i]); 
				} else if ((strcasecmp (st[i], "SRM") == 0) && (strncmp (sv[i], "1.", 2)) == 0 && ep[i]) {
					stp[i] = strdup ("srm_v1");
					sep[i] = strdup (ep[i]);
				} else if ((strcasecmp (st[i], "SRM") == 0) && (strncmp (sv[i], "2.2", 3)) == 0 && ep[i]) {
					stp[i] = strdup ("srm_v2");
					sep[i] = strdup (ep[i]);
				}
				free (st[i]);
				free (sv[i]);
				free (ep[i]);
			}
			free (st);
			free (sv);
			free (ep);
		} else if (n == 0) {
			// There were no GlueService entry...
			// ... so endpoint is hostname:port, and type is disk
			*port = ':';
			stp[0] = strdup ("disk");
			sep[0] = strdup (host_tmp);
		} else {
			free (stp);
			free (sep);
			errno = EINVAL;
			rc = -1;
		}

		if (rc == 0) {
			*se_types = stp;
			*se_endpoints = sep;
		}
	}

	return (rc);
}

/* get from the BDII the list of supported protocols with their associated
 * port number
 */

get_seap_info (const char *host, char ***access_protocol, int **port, char *errbuf, int errbufsz)
{
	static char proto_port[] = "GlueSEAccessProtocolPort";
	static char proto_type[] = "GlueSEAccessProtocolType";
	static char *template = " (& (GlueSEAccessProtocolType=*) (GlueChunkKey=GlueSEUniqueID=%s))";
	char **ap;
	char *attr;
	static char *attrs[] = {proto_type, proto_port, NULL};
	int bdii_port;
	const char *bdii_server;
	BerElement *ber;
	LDAPMessage *entry;
	char filter[128];
	int i;
	int j;
	LDAP *ld;
	int nbentries;
	int *pn;
	int rc = 0;
	LDAPMessage *reply;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (strlen (template) + strlen (host) - 2 >= sizeof (filter)) {
		snprintf (errmsg, ERRMSG_LEN, "[%s] %s: Hostname too long", gfal_remote_type, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, host);

	rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, errbuf, errbufsz);
	if (rc < 0) return -1;
	GFAL_DEBUG ("DEBUG: get_seap_info used server %s:%d\n", bdii_server, bdii_port);

	nbentries = ldap_count_entries (ld, reply);
	nbentries++;
	if ( (ap = calloc (nbentries, sizeof (char *))) == NULL ||
			(pn = calloc (nbentries, sizeof (int))) == NULL) {
		errno = ENOMEM;
		if (ap) free (ap);
		ldap_unbind (ld);
		return (-1);
	}

	for (entry = ldap_first_entry (ld, reply), i = 0;
			entry != NULL;
			entry = ldap_next_entry (ld, entry), i++) {
		for (attr = ldap_first_attribute (ld, entry, &ber);
				attr != NULL;
				attr = ldap_next_attribute (ld, entry, ber)) {
			value = ldap_get_values (ld, entry, attr);
			if (value == NULL) {
				continue;
			}
			if (strcmp (attr, "GlueSEAccessProtocolType") == 0) {
				if ( (ap[i] = strdup (value[0])) == NULL) {
					errno = ENOMEM;
					ldap_unbind (ld);
					return (-1);
				}
			} else
				pn[i] = atoi (value[0]);
			ldap_value_free (value);
		}
	}

	bdii_query_free (&ld, &reply);
	if (rc) {
		for (j = 0; j < i; j++)
			free (ap[i]);
		free (ap);
		free (pn);
	} else {
		*access_protocol = ap;
		*port = pn;
	}
	return (rc);
}
