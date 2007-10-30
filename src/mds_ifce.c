/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.42 $ $Date: 2007/10/30 12:47:20 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
#include "gfal_api.h"
static char *dn = "mds-vo-name=local,o=grid";

/* get BDII hostname and port number */
get_bdii (char *bdii_server, int buflen, int *bdii_port, char *errbuf, int errbufsz)
{
	char *bdii_env;
	char *p;

	if ((bdii_env = getenv ("LCG_GFAL_INFOSYS")) == NULL ||
			strlen (bdii_env) >= buflen) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_INFOSYS not set or invalid");
		errno = EINVAL;
		return (-1);
	}
	strcpy (bdii_server, bdii_env);
	if ((p = strchr (bdii_server, ':')) == NULL) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_INFOSYS invalid");
		errno = EINVAL;
		return (-1);
	}
	*p = '\0';
	*bdii_port = atoi (p + 1);
	return (0);
}

/* get from the BDII the CE Accesspoint for a given SE */

get_ce_ap (const char *host, char **ce_ap, char *errbuf, int errbufsz)
{
	static char ce_ap_atnm[] = "GlueCESEBindCEAccesspoint";
	static char *template = "(GlueCESEBindSEUniqueID=%s)";
	char *attr;
	static char *attrs[] = {ce_ap_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Hostname too long", bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);

	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, ce_ap_atnm);
		if (value == NULL) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: CE Accesspoint not found for host %s", bdii_server, bdii_port, host);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;

		} else {
			if ((*ce_ap = strdup (value[0])) == NULL)
				rc = -1;
			ldap_value_free (value);
		}
	} else {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: No GlueCESEBind found for host %s", bdii_server, bdii_port, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the RLS endpoints */

get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint, char *errbuf, int errbufsz)
{
	static char rls_ep[] = "GlueServiceAccessPointURL";
	static char rls_type[] = "GlueServiceType";
	static char *template = "(&(GlueServiceType=*)(GlueServiceAccessControlRule=%s))";
	char *attr;
	static char *attrs[] = {rls_type, rls_ep, NULL};
	int bdii_port;
	char bdii_server[75];
	BerElement *ber;
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	char *p;
	int rc = 0;
	LDAPMessage *reply;
	char *service_type;
	char *service_url;
	struct timeval timeout;
	char **value;
	char *vo;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
		gfal_errmsg (errbuf, errbufsz, "No VO specified, LCG_GFAL_VO not set");
		errno = EINVAL;
		return (-1);
	}
	if (strlen (template) + strlen (vo) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_VO too long");
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, vo);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
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
					if((service_type = strdup (value[0])) == NULL)
						rc = -1;
				} else {	/* GlueServiceAccessPointURL */
					if((service_url = strdup (value[0])) == NULL)
						rc = -1;
				}
				ldap_value_free (value);
			}
			else 
				rc = -1;
		}
		if (rc == 0) {
			if (strcmp (service_type, "edg-replica-metadata-catalog") == 0) {
				if ((*rmc_endpoint = strdup (service_url)) == NULL)
					rc = -1;
			} else if (strcmp (service_type, "edg-local-replica-catalog") == 0) {
				if ((*lrc_endpoint = strdup (service_url)) == NULL)
					rc = -1;
			}
		}
		free (service_type);
		free (service_url);
	}
	if (*lrc_endpoint == NULL) {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: LRC endpoint not found", bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	if (*rmc_endpoint == NULL) {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: RMC endpoint not found", bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the endpoint for the LFC */

get_lfc_endpoint (char **lfc_endpoint, char *errbuf, int errbufsz)
{
	static char ep[] = "GlueServiceAccessPointURL";
	static char *template = "(&(GlueServiceType=lcg-file-catalog)(GlueServiceAccessControlRule=%s))";
	static char *attrs[] = {ep, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char *vo;
	char errmsg[ERRMSG_LEN];

	*lfc_endpoint = NULL;
	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
		gfal_errmsg (errbuf, errbufsz, "No VO specified, LCG_GFAL_VO not set");
		errno = EINVAL;
		return (-1);
	}
	if (strlen (template) + strlen (vo) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_VO too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, vo);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 180;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	for (entry = ldap_first_entry (ld, reply);
			entry != NULL;
			entry = ldap_next_entry (ld, entry)) {
		if ((value = ldap_get_values (ld, entry, ep)) == NULL || *value == NULL)
			continue;
		if ((*lfc_endpoint = strdup (*value)) == NULL) {
			errno = ENOMEM;
			rc = -1;
		}
		ldap_value_free (value);
		break;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);

	if (rc == 0 && *lfc_endpoint == NULL) {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: LFC endpoint not found", bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}
	return (rc);
}

/* get from the BDII the root pathname to store data for a specific vo */

get_sa_root (const char *host, const char *vo, char **sa_root, char *errbuf, int errbufsz)
{
	static char sa_root_atnm[] = "GlueSARoot";
	static char *template = "(&(GlueSARoot=%s:*)(GlueChunkKey=GlueSEUniqueID=%s))";
	char *attr;
	static char *attrs[] = {sa_root_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (vo) + strlen (host) - 4 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "VO or SE hostname too long");
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, vo, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, sa_root_atnm);
		if (value == NULL) 
			goto notfound;
		if ((*sa_root = strdup (value[0] + strlen (vo) + 1)) == NULL)
			rc = -1;
		ldap_value_free (value);
	} else {
notfound:	snprintf(errmsg, ERRMSG_LEN, "%s:%d: SA Root not found for host : %s", bdii_server, bdii_port, host);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* Get from the BDII the SAPath */

get_sa_path (const char *host, const char *vo, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
	static char sa_path_atnm[] = "GlueSAPath";
	static char sa_root_atnm[] = "GlueSARoot";
	static char *template = "(&(GlueSALocalID=%s)(GlueChunkKey=GlueSEUniqueID=%s))";
	char *attr;
	static char *attrs[] = {sa_root_atnm, sa_path_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (vo) + strlen (host) - 4 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "VO or SE hostname too long");
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, vo, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	*sa_path = NULL;
	*sa_root = NULL;
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, sa_path_atnm);
		if (value != NULL) {
			/* We deal with pre-LCG 2.7.0 where SA Path was incorrect and had vo: prefix */
			if ((strncmp(value[0], vo, strlen(vo)) == 0) && (*(value[0] + strlen(vo)) == ':')) {
				if ((*sa_path = strdup (value[0] + strlen (vo) + 1)) == NULL)
					rc = -1;
			} else
				if ((*sa_path = strdup (value[0])) == NULL)
					rc = -1;
		}
		ldap_value_free (value);

		value = ldap_get_values (ld, entry, sa_root_atnm);
		if (value != NULL) {
			if ((*sa_root = strdup (value[0] + strlen (vo) + 1)) == NULL)
				rc = -1;
		}
		ldap_value_free (value);

	} else {
		/* try and get SA root via old method for pre-2.5 SEs */
		if (get_sa_root (host, vo, sa_root, errbuf, errbufsz) < 0 ) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: No GlueSA information found for SE (vo) : %s (%s)",
					bdii_server, bdii_port, host, vo);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;               
		} 

	}
	if (rc == 0) 
		if (*sa_path == NULL && *sa_root == NULL) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: Both SAPath and SARoot not set for SE (vo) : %s (%s)", 
					bdii_server, bdii_port, host, vo);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			rc = -1;
		}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}


/* get from the BDII the SE type (disk, srm_v1) */

get_se_typeandendpoint (const char *host, char **se_type, char **endpoint, char *errbuf, int errbufsz)
{
	static char se_type_atnm[] = "GlueSEName";
	static char se_type_atpt[] = "GlueSEPort";
	static char se_type_atvm[] = "GlueSchemaVersionMajor";
	static char *template = "(GlueSEUniqueID=%s)";
	static char *template1 = "(&(GlueSEUniqueID=%s)(GlueSEPort=%s))";
	char host_tmp[256];
	int len_tmp;
	char *port;
	char *attr;
	static char *attrs[] = {se_type_atnm, se_type_atpt, se_type_atvm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[300];
	LDAP *ld;
	char *p;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (se_type == NULL) {
		errno = EINVAL;
		return (-1);
	}

	len_tmp = strlen (host);
	if (strlen (template) + len_tmp - 2 >= sizeof(filter)) {
		snprintf(errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}
	strncpy (host_tmp, host, sizeof (host_tmp));
	if ((port = strchr (host_tmp, ':')) == NULL) {
		sprintf (filter, template, host_tmp);
		port = host_tmp + len_tmp;
		*port = 0;
	} else {
		*port = 0;
		sprintf (filter, template1, host_tmp, port + 1);
	}

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, se_type_atnm);
		if (value == NULL || *value == NULL) {
			errno = ENOMEM;
			return (-1);
		} else { 
			if ((p = strchr (value[0], ':')))
				p++;
			else
				p = value[0];
			if ((*se_type = strdup (p)) == NULL) {
				errno = ENOMEM;
				return (-1);
			}
			ldap_value_free (value);
		}

		if (port == NULL) {
			value = ldap_get_values (ld, entry, se_type_atpt);
			if (value == NULL || *value == NULL) {
				errno = ENOMEM;
				return (-1);
			} else if (len_tmp + strlen (value[0]) < sizeof (host_tmp)) {
				strcpy (port + 1, value[0]);
			} else {
				snprintf(errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				errno = ENAMETOOLONG;
				return (-1);
			}
			ldap_value_free (value);
		}
	} else {
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: No GlueSEName found for %s", bdii_server, bdii_port, host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);

	if (endpoint) {
		*port = ':';
		if ((*endpoint = strdup (host_tmp)) == NULL) {
			errno = ENOMEM;
			rc = -1;
		}
	}
	return (rc);
}

/* get from the BDII the supported storage types and endpoints */

get_srm_types_and_endpoints (const char *host, char ***srm_types, char ***srm_endpoints, char *errbuf, int errbufsz)
{
	static char version[] = "GlueServiceVersion";
	static char type[] = "GlueServiceType";
	static char uri[] = "GlueServiceEndpoint";
	static char *template = "(&(GlueServiceType=srm*)(GlueServiceEndpoint=*://%s*))";
	char *attr;
	static char *attrs[] = {type, version, uri, NULL};
	int bdii_port;
	char bdii_server[75];
	BerElement *ber;
	LDAPMessage *entry;
	char **ep;
	char filter[128];
	int i;
	int j;
	LDAP *ld;
	int n = 0;
	int nbentries;
	int rc = 0;
	LDAPMessage *reply;
	char **sep;
	char **st;
	char **stp;
	char **sv;
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) -1 >= sizeof(filter)) {
		snprintf(errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, host, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0, &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	nbentries = ldap_count_entries (ld, reply);
	nbentries++;
	if ((st = calloc (nbentries, sizeof(char *))) == NULL ||
			(sv = calloc (nbentries, sizeof(char *))) == NULL ||
			(ep = calloc (nbentries, sizeof(char *))) == NULL ||
			(stp = calloc (nbentries, sizeof(char *))) == NULL ||
			(sep = calloc (nbentries, sizeof(char *))) == NULL) {
		errno = ENOMEM;
		if (st) free (st);
		if (sv) free (sv);
		if (ep) free (ep);
		if (stp) free (stp);
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
			if (strcmp (attr, "GlueServiceType") == 0) {
				st[i] = strdup (value[0]);
			} else if (strcmp (attr, "GlueServiceVersion") == 0) {
				sv[i] = strdup (value[0]);
			} else {	/* GlueServiceEndpoint */
				ep[i] = strdup (value[0]);
			}
			ldap_value_free (value);
		}
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	if (rc) {
		for (j = 0; j < i; j++) {
			if (st[i]) free (st[i]);
			if (sv[i]) free (sv[i]);
			if (ep[i]) free (ep[i]);
		}
		free (st);
		free (sv);
		free (ep);
		free (stp);
		free (sep);
		*srm_types = NULL;
		*srm_endpoints = NULL;
	} else {
		*srm_types = stp;
		*srm_endpoints = sep;
		for (j = 0; j < i; ++j, ++n) {
			if ((strcmp (st[j], "srm_v1") == 0 || strcmp (st[j], "srm_v2") == 0) && ep[j]) {
				*(stp + n) = strdup (st[j]);
				*(sep + n) = strdup (ep[j]); 
			} else if ((strcasecmp (st[j], "SRM") == 0) && (strncmp (sv[j], "1.1", 3)) == 0 && ep[j]) {
				*(stp + n) = strdup ("srm_v1");
				*(sep + n) = strdup (ep[j]);
			} else if ((strcasecmp (st[j], "SRM") == 0) && (strncmp (sv[j], "2.2", 3)) == 0 && ep[j]) {
				*(stp + n) = strdup ("srm_v2");
				*(sep + n) = strdup (ep[j]);
			}
			free (st[j]);
			free (sv[j]);
			free (ep[j]);
		}
		free (st);
		free (sv);
		free (ep);	
	}

	if (*srm_types[0] == NULL || *srm_endpoints[0] == NULL) {
		free (stp);
		free (sep);
		*srm_types = NULL;
		*srm_endpoints = NULL;
		errno = ENOMEM;
		rc = -1;
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
	static char *template = "(&(GlueSEAccessProtocolType=*)(GlueChunkKey=GlueSEUniqueID=%s))";
	char **ap;
	char *attr;
	static char *attrs[] = {proto_type, proto_port, NULL};
	int bdii_port;
	char bdii_server[75];
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
	struct timeval timeout;
	char **value;
	char errmsg[ERRMSG_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		snprintf(errmsg, ERRMSG_LEN, "%s: Hostname too long", host);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Refused", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
					&timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII Connection Timeout", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
		} else {
			snprintf(errmsg, ERRMSG_LEN, "%s:%d: BDII ERROR: %s", bdii_server, bdii_port, 
					ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
		}
		return (-1);
	}
	nbentries = ldap_count_entries (ld, reply);
	nbentries++;
	if ((ap = calloc (nbentries, sizeof(char *))) == NULL ||
			(pn = calloc (nbentries, sizeof(int))) == NULL) {
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
				if ((ap[i] = strdup (value[0])) == NULL) {
					errno = ENOMEM;
					ldap_unbind (ld);
					return (-1);
				}
			} else
				pn[i] = atoi (value[0]);
			ldap_value_free (value);
		}
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
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
