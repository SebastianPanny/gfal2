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
#include "Cns_constants.h"
#include "test_helpers.h"

char error_msg[1024];

const char *root_path = "/grid/dteam/test/catalog-test";
const char *server_name = "foo.example.com";

char errbuf[ERRBUFSZ];


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
helper_make_test_root() {
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
  if((guid = guidfromlfn(root_path, errbuf, ERRBUFSZ)) == NULL) {
    if(errno == ENOENT) {
      helper_make_guid(root_guid);
      if((lfc_mkdirg(root_path, root_guid, 0775, errbuf, ERRBUFSZ)) < 0) {
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
int helper_remove_lfn(const char* lfn) {
  char *guid;
  char **lfns;
  char **replicas;
  char **p;

  if((guid = guidfromlfn(lfn, errbuf, ERRBUFSZ)) == NULL) {
    return (0);
  }

  if((replicas = surlsfromguid(guid, errbuf, ERRBUFSZ)) != NULL) {
    for(p = replicas; *p != NULL; p++) {
      if(unregister_pfn(guid, *p, errbuf, ERRBUFSZ) < 0) {
	sprintf(error_msg, "Could not unregister surl %s : %s\n", 
		*p, strerror(errno));
	fail(error_msg);
      }
      free(*p);
    }
    free(replicas);
  }
  
  if((lfns = lfnsforguid(guid, errbuf, ERRBUFSZ)) != NULL) {
    for(p = lfns; *p != NULL; p++) {
      if(strcmp(lfn, *p) != 0) {
	if(unregister_alias(guid, *p, errbuf, ERRBUFSZ) < 0) {
	  sprintf(error_msg, "Could not unregister lfn %s : %s\n", 
		  *p, strerror(errno));
	  fail(error_msg);
	}
      }
      free(*p);
    }
    free(lfns);
  }

  unregister_alias(guid, lfn, errbuf, ERRBUFSZ);
  free(guid);
  return 0;
}

/* helper_remove_surl : remove a surl if it exists in the catalog */
int helper_remove_surl(const char* surl) {
  char *guid;

  if((guid = guidforpfn(surl, errbuf, ERRBUFSZ)) == NULL) {
    return (0);
  }

  if(unregister_pfn(guid, surl, errbuf, ERRBUFSZ) < 0) {
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

