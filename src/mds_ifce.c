/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.1.1.1 $ $Date: 2003/11/19 12:56:29 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ldap.h>
static char *dn = "mds-vo-name=local,o=grid";

/* get from the BDII the RLS endpoints */

get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint)
{
	static char *rls_ep = "GlueServiceAccessPointURL";
	static char *rls_type = "GlueServiceType";
	static char *rls_vo = "GlueServiceAccessControlRule";
	static char *template = "(&(GlueServiceType=*)(GlueServiceAccessControlRule=%s))";
	char *attr;
	char *attrs[] = {rls_type, rls_ep, NULL};
	int bdii_port;
	char *bdii_server;
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

	if ((bdii_server = getenv ("LCG_GFAL_INFOSYS")) == NULL ||
	    (p = strchr (bdii_server, ':')) == NULL) {
		errno = EINVAL;
		return (-1);
	}
	*p = '\0';
	bdii_port = atoi (p + 1);
	if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if (strlen (template) + strlen (vo) - 2 >= sizeof(filter)) {
		errno = EINVAL;
		return (-1);
	}
	sprintf (filter, template, vo);

	if ((ld = ldap_init (bdii_server, bdii_port)) == NULL)
		return (-1);
	if (ldap_simple_bind_s (ld, "", "") != LDAP_SUCCESS) {
		ldap_unbind (ld);
		return (-1);
	}
	timeout.tv_sec = 5;
	if (ldap_search_st (ld, dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
	    &timeout, &reply) != LDAP_SUCCESS) {
		ldap_unbind (ld);
		return (-1);
	}
	for (entry = ldap_first_entry (ld, reply);
	     entry != NULL;
	     entry = ldap_next_entry (ld, entry)) {
		for (attr = ldap_first_attribute (ld, entry, &ber);
		     attr != NULL;
		     attr = ldap_next_attribute (ld, entry, ber)) {
			value = ldap_get_values (ld, entry, attr);
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
	ldap_unbind (ld);
	return (rc);
}
