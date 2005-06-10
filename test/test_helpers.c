/*
 * Copyright (C) 2004 by CERN
 * All rights reserved
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <check.h>
#include <uuid/uuid.h>

#include "gfal_api.h"
#include "lfc_api.h"
#include "Cns_constants.h"
#include "test_helpers.h"

char error_msg[1024];

const char *root_path = "/grid/dteam/test/catalog-test";
const char *server_name = "foo.example.com";


/** helper_make_guid : generate a unique ID into a predefined buffer.  It
    must be at least CA_MAXGUIDLEN+1 in length */
void
helper_make_guid(char *guid_buf) { 
  uuid_t uuid;
  uuid_generate(uuid);
  uuid_unparse(uuid, guid_buf);
}

/** helper_make_path : create a pathname below the test root directory.  buf
    must be at CA_MAXPATHLEN+1 long.*/
void 
helper_make_lfn(char *buf, const char* path) {
  if(strlen(root_path) + strlen(path) +1 > CA_MAXPATHLEN) {
    fail("Internal Error : path is too long for buffer");
  }
  sprintf(buf, "%s/%s", root_path, path);
}

void
helper_make_surl(char *buf, const char *path) {
  sprintf(buf, "srm://%s/%s", server_name, path);
}

void
helper_make_test_root(char *errbuf, int errbufsz) {
  char *guid;
  char root_guid[CA_MAXGUIDLEN+1];
  char *cat_type;

  if(get_cat_type(&cat_type) <0 ) {
    fail("Could not get catalog type");
  }
  if(strcmp(cat_type, "lfc") != 0) {
    free(cat_type);
    return;
  }

  free(cat_type);
  /* If root doesn't exist,  make it */
  if((guid = guidfromlfn(root_path, errbuf, errbufsz)) == NULL) {
    if(errno == ENOENT) {
      helper_make_guid(root_guid);
      if((lfc_mkdirp(root_path, 0775, errbuf, errbufsz)) < 0) {
	sprintf(error_msg, "Could not create test root : %s : %s\n",
		root_path, strerror(errno));
	fail(error_msg);
      }
    } else {
	sprintf(error_msg, "Could not get lfn from guid : %s : %s\n",
		guid, strerror(errno));
	fail(error_msg);
    }
  }
  free(guid);
  return;
}

/* helper_remove_lfn : remove an lfn, and all replicas from a catalog */
int helper_remove_lfn(const char* lfn, char* errbuf, int errbufsz) {
  char *guid;
  char **lfns;
  char **replicas;
  char **p;
  int has_error = 0;

  if((guid = guidfromlfn(lfn, errbuf, errbufsz)) == NULL) {
    return (0);
  }

  if((replicas = surlsfromguid(guid, errbuf, errbufsz)) != NULL) {
    for(p = replicas; *p != NULL; p++) {
      if(has_error == 0) {
	if(unregister_pfn(guid, *p, errbuf, errbufsz) < 0) {
	  sprintf(error_msg, "Could not unregister surl %s : %s\n", 
		  *p, strerror(errno));
	  has_error = 1;
	}
      }
      free(*p);
    }
    free(replicas);
    if(has_error) {
      fail(error_msg);
    }
  }
  
  if((lfns = lfnsforguid(guid, errbuf, errbufsz)) != NULL) {
    for(p = lfns; *p != NULL; p++) {
      if(strcmp(lfn, *p) != 0) {
	if(unregister_alias(guid, *p, errbuf, errbufsz) < 0) {
	  sprintf(error_msg, "Could not unregister lfn %s : %s\n", 
		  *p, strerror(errno));
	  fail(error_msg);
	}
      }
      free(*p);
    }
    free(lfns);
  }

  unregister_alias(guid, lfn, errbuf, errbufsz);
  free(guid);
  return 0;
}

/* helper_remove_surl : remove a surl if it exists in the catalog */
int helper_remove_surl(const char* surl, char *errbuf, int errbufsz) {
  char *guid;

  if((guid = guidforpfn(surl, errbuf, errbufsz)) == NULL) {
    return (0);
  }

  if(unregister_pfn(guid, surl, errbuf, errbufsz) < 0) {
    if(errno == ENOENT) {
      return (-1);
    }
    sprintf(error_msg, "Could not unregister surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
  free(guid);
  return 0;
}

int setenviron (const char *name, const char *value) {
  char *env;

  if((env = (char *)malloc (strlen(name) + strlen(value)+ 2)) == NULL) {
    fail("Could not malloc space for env");
  }
  sprintf (env, "%s=%s", name, value);
  if(putenv (env) < 0) {
    fail("Could not set env");
  }
  return (0);
}

int clearenviron (const char *name) {
  char *env;

  if((env = (char *)malloc (strlen(name)+ 1)) == NULL) {
    fail("Could not malloc space for env");
  }
  sprintf (env, "%s", name);
  if(putenv (env) < 0) {
    fail("Could not set env");
  }
  return (0);
}

