/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.23 $ $Date: 2005/12/12 10:17:41 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
static char *dn = "mds-vo-name=local,o=grid";

#define ERROR_STR_LEN 255

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

get_ce_ap (const char *host, char **ce_ap)
{
	return (get_ce_apx (host, ce_ap, NULL, 0));
}

get_ce_apx (const char *host, char **ce_ap, char *errbuf, int errbufsz)
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
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
	        snprintf(error_str, ERROR_STR_LEN, "BDII Hostname too long: %s:%d", bdii_server, bdii_port);
		gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
	return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
		        errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				  ldap_err2string(rc));
		        gfal_errmsg(errbuf, errbufsz, error_str);
		        errno = EINVAL;
		}
		return (-1);

	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, ce_ap_atnm);
		if (value == NULL) {
 	                snprintf(error_str, ERROR_STR_LEN, "CE Accesspoint not found for host : %s", host);
			gfal_errmsg (errbuf, errbufsz, error_str);
			errno = EINVAL;
			rc = -1;

		} else {
		  if ((*ce_ap = strdup (value[0])) == NULL)
		    rc = -1;
		  ldap_value_free (value);
		}
	} else {
 	        snprintf(error_str, ERROR_STR_LEN, "No GlueCESEBind found for host : %s", host);
 	        gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the RLS endpoints */

get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint)
{
	return (get_rls_endpointsx (lrc_endpoint, rmc_endpoint, NULL, 0));
}

get_rls_endpointsx (char **lrc_endpoint, char **rmc_endpoint, char *errbuf, int errbufsz)
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
	int rc = 0;
	LDAPMessage *reply;
	char *service_type;
	char *service_url;
	struct timeval timeout;
	char **value;
	char *vo;
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_VO not set");
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
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
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
		gfal_errmsg (errbuf, errbufsz, "LRC endpoint not found");
		errno = EINVAL;
		rc = -1;
	}
	if (*rmc_endpoint == NULL) {
		gfal_errmsg (errbuf, errbufsz, "RMC endpoint not found");
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
	static char type[] = "GlueServiceType";
	static char *template = "(&(GlueServiceType=*)(GlueServiceAccessControlRule=%s))";
	char *attr;
	static char *attrs[] = {type, ep, NULL};
	int bdii_port;
	char bdii_server[75];
	BerElement *ber;
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	char *service_type;
	char *service_url;
	struct timeval timeout;
	char **value;
	char *vo;
	char error_str[ERROR_STR_LEN];
	
	*lfc_endpoint = NULL;
	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
		gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_VO not set");
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
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if (ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
		        errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
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
			if (value == NULL) {
				rc = -1;
				continue;
			}
			if (strcmp (attr, "GlueServiceType") == 0) {
				if((service_type = strdup (value[0])) == NULL)
					rc = -1;
			} else {	/* GlueServiceAccessPointURL */
				if((service_url = strdup (value[0])) == NULL)
					rc = -1;
			}
			ldap_value_free (value);
		}
		if (rc == 0) {
			if (strcmp (service_type, "lcg-file-catalog") == 0) {
			  if ((*lfc_endpoint = strdup (service_url)) == NULL)
			    rc = -1;
			}
		}
		free (service_type);
		free (service_url);
	}
	if (*lfc_endpoint == NULL) {
		gfal_errmsg (errbuf, errbufsz, "LFC endpoint not found");
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the root pathname to store data for a specific vo */

get_sa_root (const char *host, const char *vo, char **sa_root)
{
	return (get_sa_rootx (host, vo, sa_root, NULL, 0));
}

get_sa_rootx (const char *host, const char *vo, char **sa_root, char *errbuf, int errbufsz)
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
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (vo) + strlen (host) - 4 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "vo or host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, vo, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
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
notfound:	snprintf(error_str, ERROR_STR_LEN, "SA Root not found for host : %s", host);
                gfal_errmsg (errbuf, errbufsz, error_str);
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
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (vo) + strlen (host) - 4 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "vo or host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, vo, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
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
                snprintf(error_str, ERROR_STR_LEN, "No GlueSA information found for SE (vo) : %s (%s)", host, vo);
                gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		rc = -1;
	}
        if (rc == 0)
                if (*sa_path == NULL && *sa_root == NULL) {
                        snprintf(error_str, ERROR_STR_LEN, 
                                 "Both SAPath and SARoot not set for SE (vo) : %s (%s)", 
                                 host, vo);
                        gfal_errmsg (errbuf, errbufsz, error_str);
                        errno = EINVAL;
                        rc = -1;
                }
        ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}


/* get from the BDII the SE endpoint */

get_se_endpoint (const char *host, char **se_endpoint)
{
	return (get_se_endpointx (host, se_endpoint, NULL, 0));
}

get_se_endpointx (const char *host, char **se_endpoint, char *errbuf, int errbufsz)
{
	static char se_ep_atnm[] = "GlueServiceURI";
	static char *template = "(&(GlueServiceURI=*%s*)(GlueServiceType=srm_v1))";
	char *attr;
	static char *attrs[] = {se_ep_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char error_str[ERROR_STR_LEN];
	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = EINVAL;
		}
		return (-1);
	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, se_ep_atnm);
		if (value == NULL) {
		        snprintf(error_str, ERROR_STR_LEN, "SE (SRM) endpoint not set for host : %s", host);
			gfal_errmsg (errbuf, errbufsz, "SE (SRM) endpoint not set for host : %s");
			errno = EINVAL; 	
			rc = -1;
		}
		else {
			if ((*se_endpoint = strdup (value[0])) == NULL)
				rc = -1;
			ldap_value_free (value);
		}
	} else {
       	        snprintf(error_str, ERROR_STR_LEN, "SE (SRM) service not found for host : %s", host);
		gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the SE port */

get_se_port (const char *host, int *se_port)
{
	return (get_se_portx (host, se_port, NULL, 0));
}

get_se_portx (const char *host, int *se_port, char *errbuf, int errbufsz)
{
	static char se_port_atnm[] = "GlueSEPort";
	static char *template = "(GlueSEUniqueID=%s)";
	char *attr;
	static char *attrs[] = {se_port_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = EINVAL;
		}
		return (-1);
	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, se_port_atnm);
		if (value == NULL) {
 	                snprintf(error_str, ERROR_STR_LEN, "SE port not found for host : %s", host);
			gfal_errmsg (errbuf, errbufsz, error_str);
			errno = EINVAL;
			rc = -1;
		} else {
 		        *se_port = atoi (value[0]);
			ldap_value_free (value);
		}
	} else {
 	        snprintf(error_str, ERROR_STR_LEN,"No information found for SE : %s", host);
                gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the SE type (disk, srm_v1) */

get_se_type (const char *host, char **se_type)
{
	return (get_se_typex (host, se_type, NULL, 0));
}

get_se_typex (const char *host, char **se_type, char *errbuf, int errbufsz)
{
	static char se_type_atnm[] = "GlueSEName";
	static char *template = "(GlueSEUniqueID=%s)";
	char *attr;
	static char *attrs[] = {se_type_atnm, NULL};
	int bdii_port;
	char bdii_server[75];
	LDAPMessage *entry;
	char filter[128];
	LDAP *ld;
	char *p;
	int rc = 0;
	LDAPMessage *reply;
	struct timeval timeout;
	char **value;
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = EINVAL;
		}
		return (-1);
	}
	entry = ldap_first_entry (ld, reply);
	if (entry) {
		value = ldap_get_values (ld, entry, se_type_atnm);
		if (value == NULL) {
		  snprintf(error_str, ERROR_STR_LEN,  "SE type not found for host : %s", host);
		  gfal_errmsg (errbuf, errbufsz, error_str);
		  errno = EINVAL;
		  rc = -1;

		} else { 
 		  if ((p = strchr (value[0], ':')))
		    p++;
		  else
		    p = value[0];
		  if ((*se_type = strdup (p)) == NULL)
		    rc = -1;
		  ldap_value_free (value);
		}
	} else {
	        snprintf(error_str, ERROR_STR_LEN,  "No information found for SE : %s", host);
                gfal_errmsg (errbuf, errbufsz, error_str);
		errno = EINVAL;
		rc = -1;
	}
	ldap_msgfree (reply);
	ldap_unbind (ld);
	return (rc);
}

/* get from the BDII the list of supported protocols with their associated
 * port number
 */

get_seap_info (const char *host, char ***access_protocol, int **port)
{
	return (get_seap_infox (host, access_protocol, port, NULL, 0));
}

get_seap_infox (const char *host, char ***access_protocol, int **port, char *errbuf, int errbufsz)
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
	char error_str[ERROR_STR_LEN];

	if (get_bdii (bdii_server, sizeof(bdii_server), &bdii_port, errbuf, errbufsz) < 0)
		return (-1);
	if (strlen (template) + strlen (host) - 2 >= sizeof(filter)) {
		gfal_errmsg (errbuf, errbufsz, "host too long");
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, host);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		snprintf(error_str, ERROR_STR_LEN, "BDII Connection Refused: %s:%d", bdii_server, bdii_port);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = ECONNREFUSED;
		return (-1);
	}
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	if ((rc = ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply)) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
  		        snprintf(error_str, ERROR_STR_LEN, "BDII Connection Timeout: %s:%d", bdii_server, bdii_port);
	 	        gfal_errmsg(errbuf, errbufsz, error_str);
			errno = ETIMEDOUT;
		} else {
		        snprintf(error_str, ERROR_STR_LEN, "BDII ERROR: %s:%d %s", bdii_server, bdii_port, 
				 ldap_err2string(rc));
			gfal_errmsg(errbuf, errbufsz, error_str);
			errno = EINVAL;
		}
		return (-1);
	}
	nbentries = ldap_count_entries (ld, reply);
	nbentries++;
	if ((ap = calloc (nbentries, sizeof(char *))) == NULL) {
		ldap_unbind (ld);
		return (-1);
	}
	if ((pn = calloc (nbentries, sizeof(int))) == NULL) {
		free (ap);
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
				rc = -1;
				continue;
			}
			if (strcmp (attr, "GlueSEAccessProtocolType") == 0) {
				if ((ap[i] = strdup (value[0])) == NULL)
					rc = -1;
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
