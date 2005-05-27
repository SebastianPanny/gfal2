/*
 * Copyright (C) 2004 by CERN
 * All rights reserved
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "lfc_api.h"
#include "gfal_api.h"

#include "Cns_constants.h"

#include "test_helpers.h"

#define DEFAULT_SIZE 12345678

/** catalog_tests : These test the catalog operations  */

char *errbuf;

char error_msg[1024];
char file_path[CA_MAXPATHLEN+1];

char file_guid[CA_MAXGUIDLEN+1];

void setup_common() {
  char *name = "catalog_tests";
  
  /* this is needed for getbestfile to work out the default se */
  setenviron("LCG_GFAL_VO", "test");
  /* setup errbuf */
  if((errbuf = (char*) malloc((ERRBUFSZ+1) * sizeof(char))) == NULL) {
    return;
  }
  
  /* add a sample lfn and guid for testing replica functions */
  helper_make_guid(file_guid);
  helper_make_lfn(file_path, name);

  helper_remove_lfn(file_path, errbuf, ERRBUFSZ);

  helper_make_test_root(errbuf, ERRBUFSZ);
  if(create_alias(file_guid, file_path, DEFAULT_SIZE, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not create file : %s : %s : %s\n", 
	    file_path, file_guid, strerror(errno));
    fail(error_msg);
  }
}

  void teardown_common() {
    free(errbuf);
  }

/** setup_edg_catalog : put the approriate env for the EDG LRC/RMC */
void setup_edg_catalog() {
  char *cat_env = "LCG_CATALOG_TYPE=edg";
  if(putenv(cat_env) < 0) {
    sprintf(error_msg, "Could not putenv(catalog_type=edg) : %s\n", 
	    strerror(errno));
    fail(error_msg);
  }
  
  setup_common();
}

/** setup_lfc_catalog : put the appropriate env for the LCG LFC */
void setup_lfc_catalog() {
  char *cat_env = "LCG_CATALOG_TYPE=lfc";  
  if(putenv(cat_env) < 0) {
    sprintf(error_msg, "Could not putenv(catalog_type=lfc) : %s\n", 
	    strerror(errno));
    fail(error_msg);
  }

  setup_common();
}

/** test_get_cat_type : check that get_cat_type() obeys the environment 
    variable */
START_TEST(test_get_cat_type) {
  char *cat_type;
  char *the_cat = "foo";
  char *other_cat = "bar";
  char cat_env[32];
  
  sprintf(cat_env, "LCG_CATALOG_TYPE=%s", the_cat);
  if(putenv(cat_env) < 0) {
    sprintf(error_msg, "Could not putenv(catalog_type=foo) : %s\n", 
	    strerror(errno));
    fail(error_msg);
  }

  if(get_cat_type(&cat_type) < 0 ) {
    sprintf(error_msg, "Could not get catalog type : %s\n", strerror(errno));
    fail(error_msg);
  }
  fail_unless(strcmp(the_cat, cat_type) == 0, "Incorrect catalog type");
  free(cat_type);

  sprintf(cat_env, "LCG_CATALOG_TYPE=%s", other_cat);
  if(putenv(cat_env) < 0) {
    sprintf(error_msg, "Could not putenv(catalog_type=bar) : %s\n", 
	    strerror(errno));
    fail(error_msg);
  }

  if(get_cat_type(&cat_type) < 0 ) {
    sprintf(error_msg, "Could not get catalog type : %s\n", strerror(errno));
    fail(error_msg);
  }
 fail_unless(strcmp(other_cat, cat_type) == 0, "Incorrect catalog type");
  free(cat_type);

}END_TEST

/** test_get_default_catalog : check that the default catalog is EDG */
START_TEST(test_get_default_catalog) {
  char *cat_type;
  char *cat_env = "LCG_CATALOG_TYPE";
  
  /* remove from env, if it's there */
  if(putenv(cat_env) < 0) {
    sprintf(error_msg, "Could not putenv(LCG_CATALOG_TYPE=null) : %s\n", 
	    strerror(errno));
    fail(error_msg);
  }

  if(get_cat_type(&cat_type) < 0 ) {
    sprintf(error_msg, "Could not get catalog type : %s\n", strerror(errno));
    fail(error_msg);
  }
  fail_unless(strcmp("edg", cat_type) == 0, "Incorrect catalog type");
  free(cat_type);

}END_TEST

/** test_guid_exists : check guid_exists works for existing and non-existing
    guids */
START_TEST(test_guid_exists) {
  int exists;
  char nexist_guid[CA_MAXGUIDLEN+1];

  if((exists = guid_exists(file_guid, errbuf, ERRBUFSZ)) < 0) {
    sprintf(error_msg, "Could not check guid existence  %s : %s\n", 
	    file_guid, strerror(errno));
    fail(error_msg);
  }
  fail_unless(exists, "guid should exist");
  
  /* now try a non-existent one */
  helper_make_guid(nexist_guid);
  if((exists = guid_exists(nexist_guid, errbuf, ERRBUFSZ)) < 0) {
    sprintf(error_msg, "Could not check guid existence  %s : %s\n", 
	    nexist_guid, strerror(errno));
    fail(error_msg);
  }
  fail_unless(!exists, "guid should not exist");

}END_TEST

/** test_create_lfn : test we can register a new LFN. */
START_TEST(test_create_lfn) {
  char guid[CA_MAXGUIDLEN+1];
  char lfn[CA_MAXPATHLEN+1];
  char *name = "test_create_lfn";
  char **lfns;
  char **p;
  int got = 0;

  helper_make_guid(guid);
  helper_make_lfn(lfn, name);
  helper_remove_lfn(lfn, errbuf, ERRBUFSZ);

  /* and register */
  if(create_alias(guid, lfn, DEFAULT_SIZE, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not create lfn  %s : %s : %s\n", 
	    lfn, errbuf, strerror(errno) );
    fail(error_msg);
  }

  /* check it's there */
  if((lfns = lfnsforguid(guid, errbuf, ERRBUFSZ)) == NULL) {
    sprintf(error_msg, "Could not get lfns for guid  %s : %s\n", 
	    guid, strerror(errno));
    fail(error_msg);
  }    
  for(p = lfns; *p != NULL; p++) {
    if(strcmp(lfn, *p) == 0) {
      got = 1;
    }
    free(*p);
  }
  free(lfns);
  fail_unless(got == 1, "should have lfn returned");
}END_TEST

/** test_register_before_create : we should get ENOENT if we try and register
    to a guid, which isn't attached to a master lfn already */
START_TEST(test_register_before_create) {
  char lfn[CA_MAXPATHLEN+1];
  char guid[CA_MAXGUIDLEN+1];
  char *name = "test_register_before_create";

  helper_make_guid(guid);
  helper_make_lfn(lfn, name);
  if(register_alias(guid, lfn, errbuf, ERRBUFSZ) < 0) {
    if(errno == ENOENT) {
      return;
    }
    sprintf(error_msg, "Could not register lfn  %s : %s\n", 
	    lfn, strerror(errno));
    fail(error_msg);

  }
  fail("Shouldn't be able to register lfn before a create");
}END_TEST

/** test_multiple_register_lfn : test we can register serveral lfns for a
    single guid. We have a master lfn, and them some more which we do
    register_alias on */
START_TEST(test_multiple_register_lfn) {
  char guid[CA_MAXGUIDLEN+1];
  char lfn[CA_MAXPATHLEN+1];
  char buf[CA_MAXPATHLEN+1];
  char *name = "test_multiple_create_lfn";

  char **lfns;
  char **p;
  int i;

  helper_make_guid(guid);
  helper_make_lfn(lfn, name);
  helper_remove_lfn(lfn, errbuf, ERRBUFSZ);

  /* and register */

  if(create_alias(guid, lfn, DEFAULT_SIZE, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not create master lfn  %s : %s\n", 
	    lfn, strerror(errno));
    fail(error_msg);
  }

  for (i = 0; i < 10 ; ++i) {
    sprintf(buf, "%s%d", lfn, i);
    if(register_alias(guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not register lfn  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* check it's there */
  if((lfns = lfnsforguid(guid, errbuf, ERRBUFSZ)) == NULL) {
    sprintf(error_msg, "Could not get lfns for guid  %s : %s\n", 
	    guid, strerror(errno));
    fail(error_msg);
  }    
  i = 0;
  for(p = lfns; *p != NULL; p++) {
    ++i;
    free(*p);
  }
  free(lfns);
  fail_unless(i == 11, "Should have 11 results");
}END_TEST


/** test_registerpfn : test we can register a surl.  For the LFC, this relies
    on the guid being in the catalog, attached to a LFN */
START_TEST(test_registerpfn) {
  char surl[CA_MAXSFNLEN+1];
  char *surl_name = "foo/test_registerpfn";
  char *guid;
  
  helper_make_surl(surl, surl_name);
  helper_remove_surl(surl, errbuf, ERRBUFSZ);

  /* and register */
  if(register_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }

  if((guid = guidforpfn(surl, errbuf, ERRBUFSZ)) == NULL) {
    sprintf(error_msg, "Could not get guid for surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
  fail_unless(strcmp(guid, file_guid) == 0, "same guid expected back");
  free(guid);
}END_TEST

/** test_registerpfn_exists : test that we get an ENOENT error trying to
    register an existing surl on a non-existing guid*/
START_TEST(test_registerpfn_exists) {
  char guid[CA_MAXGUIDLEN+1];
  char guid2[CA_MAXGUIDLEN+1];
  char surl[CA_MAXSFNLEN+1];
  char *surl_name = "foo/test_registerpfn";

  helper_make_guid(guid);
  helper_make_surl(surl, surl_name);

  helper_remove_surl(surl, errbuf, ERRBUFSZ);

  /* register */
  if(register_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }


  /* and register again on a different guid - should get a ENOENT for the
     guid */
  helper_make_guid(guid2);
  if(register_pfn(guid2, surl, errbuf, ERRBUFSZ) < 0) {
    if(errno == ENOENT) {
      return;
    }
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
}END_TEST

/** test_registerpfn_exists : test that we get an EEXIST error trying to
    register an existing surl on an existing guid */
START_TEST(test_registerpfn_guid_exists) {
  char guid[CA_MAXGUIDLEN+1];
  char guid2[CA_MAXGUIDLEN+1];
  char surl[CA_MAXSFNLEN+1];
  char lfn[CA_MAXPATHLEN+1];
  char *name="test_registerpfn_guid_exists";
  char *surl_name = "foo/test_registerpfn";

  helper_make_guid(guid);
  helper_make_surl(surl, surl_name);

  helper_remove_surl(surl, errbuf, ERRBUFSZ);

  /* register */
  if(register_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }


  /* Make a second lfn, and and register on its guid - should get a EEXIST
     for the guid */
  helper_make_guid(guid2);
  helper_make_lfn(lfn, name);
  helper_remove_lfn(lfn, errbuf, ERRBUFSZ);
  if(create_alias(guid2, lfn, DEFAULT_SIZE, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not create %s : %s\n", 
	    lfn, strerror(errno));
    fail(error_msg);
  }
  if(register_pfn(guid2, surl, errbuf, ERRBUFSZ) < 0) {
    if(errno == EEXIST) {
      return;
    }
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
}END_TEST


/** test_unregisterpfn : test we can register and then unregister a surl */
START_TEST(test_unregisterpfn) {
  char surl[CA_MAXSFNLEN+1];
  char *surl_name = "foo/test_unregisterpfn";
  char *guid;
  
  helper_make_surl(surl, surl_name);
  helper_remove_surl(surl, errbuf, ERRBUFSZ);

  /* and register */
  if(register_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not register surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }

  /* and unregister */
  if(unregister_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not unregister surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }

  if((guid = guidforpfn(surl, errbuf, ERRBUFSZ)) == NULL) {
    if(errno == ENOENT) {
      return;
    }
    sprintf(error_msg, "Could not get guid for surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
  free(guid);
  fail("Should not have been a guid for the surl - it was unregistered");
}END_TEST

/** test_unregisterpfn_nexist : test we get no error if we try and unregister a
    non-existent surl */
START_TEST(test_unregisterpfn_nexist) {
  char surl[CA_MAXSFNLEN+1];
  char *surl_name = "foo/this_does_not_exist";
  
  helper_make_surl(surl, surl_name);
  helper_remove_surl(surl, errbuf, ERRBUFSZ);

  /* and unregister */
  if(unregister_pfn(file_guid, surl, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Could not unregister surl %s : %s\n", 
	    surl, strerror(errno));
    fail(error_msg);
  }
  return;
}END_TEST

/** test_getfilesize : check we can get the filesize for a guid */
START_TEST(test_getfilesize) {
  long long guid_size;
  
  if(getfilesizeg(file_guid, &guid_size, errbuf, ERRBUFSZ) < 0) {
    sprintf(error_msg, "Can't get file size on guid %s : %s\n", 
	    file_guid, strerror(errno));
    fail(error_msg);
  }
  fail_unless(DEFAULT_SIZE == guid_size, "Correct size returned");
}END_TEST

void
free_list(char **p, int i) {
  char **cp;
  int j = 0;

  if(i == 0 && p == NULL)
    return;
  fail_unless(p != NULL, "p must be valid pointer");
  for(cp = p; *cp != NULL; ++cp, ++j)
    free(*cp);
  fail_unless(i == j, "correct number of entries");
  free(p);
}

/** test_replica_exists_unknown_guid : check that we get -1 on a unknown
    guid */
START_TEST(test_replica_exists_unknown_guid) {
  char unknown_guid[CA_MAXGUIDLEN+1];
  int num;

  helper_make_guid(unknown_guid);

  num = replica_exists(unknown_guid, errbuf, ERRBUFSZ);
  fail_unless(num == 0, "Expected 0 from replica_exists on unknown guid");
}END_TEST

/** test_replica_exists : check that we can add and remove surls to/from a guid
    and the count stays consistent */
START_TEST(test_replica_exists) {
  char buf[CA_MAXSFNLEN+1];
  char base_surl[CA_MAXSFNLEN+1];
  char *path="foo/test_surlsfromguid";
  int i;
  int num=0;

  /* check that the list is initially empty */
  if((num = replica_exists(file_guid, errbuf, ERRBUFSZ)) < 0) {
    fail("There should be no surls initially");
  }
  fail_unless(num ==0, "Should be no surls");

  /* now add some, and check the size */
  helper_make_surl(base_surl, path);
  for (i = 1; i < 10 ; ++i) {
    sprintf(buf, "%s%d", base_surl, i);
    if(register_pfn(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not register surl  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
    if((num = replica_exists(file_guid, errbuf, ERRBUFSZ)) < 0) {
      sprintf(error_msg, "Could not get surls for guid  %s : %s\n", 
	      file_guid, strerror(errno));
      fail(error_msg);
    }
    fail_unless(num == 1, "Should be surls.");
  }


  /* and remove them again */
  for (i = 1; i < 10 ; ++i) {
    if((num = replica_exists(file_guid, errbuf, ERRBUFSZ)) < 0) {
      sprintf(error_msg, "Could not count surls for guid  %s : %d : %s\n", 
	      file_guid, i, strerror(errno));
      fail(error_msg);
    }
    fail_unless(num == 1, "Should be surls on delete before last one");
    sprintf(buf, "%s%d", base_surl, i);
    if(unregister_pfn(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not unregister surl  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* check that the list is finally empty */
  if((num = replica_exists(file_guid, errbuf, ERRBUFSZ)) < 0) {
    fail("There should be no surls finally");
  }
  fail_unless(num == 0, "there should be no surls at end");
}END_TEST


/** test_surlsfromguid_unknown_guid : check that we get NULL on a unknown
    guid */
START_TEST(test_surlsfromguid_unknown_guid) {
  char unknown_guid[CA_MAXGUIDLEN+1];
  char **p;

  helper_make_guid(unknown_guid);

  p = surlsfromguid(unknown_guid, errbuf, ERRBUFSZ);
  if(p == NULL || *p != NULL) {
    fail("Expected empty list from surlsfromguid");
    if (p != NULL)
      free_list(p, sizeof(p)/sizeof(char*));
  }
}END_TEST

/** test_surlsfromguid : check that we can add and remove surls to/from a guid
    and the list stays consistent */
START_TEST(test_surlsfromguid) {
  char **p;
  char buf[CA_MAXSFNLEN+1];
  char base_surl[CA_MAXSFNLEN+1];
  char *path="foo/test_surlsfromguid";
  int i;

  /* check that the list is initially empty */
  if((p = surlsfromguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
    fail("NULL returned by surlsfromguid");
  }
  if(*p != NULL) {
    fail("there should be no surls initially");
  }
  
  /* now add some, and check the size */
  helper_make_surl(base_surl, path);
  for (i = 1; i < 10 ; ++i) {
    sprintf(buf, "%s%d", base_surl, i);
    if(register_pfn(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not register surl  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
    if((p = surlsfromguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Could not get surls for guid  %s : %s\n", 
	      file_guid, strerror(errno));
      fail(error_msg);
    }
    free_list(p, i);
  }


  /* and remove them again */
  for (i = 1; i < 10 ; ++i) {
    if((p = surlsfromguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Could not get surls for guid  %s : %d : %s\n", 
	      file_guid, i, strerror(errno));
      fail(error_msg);
    }
    free_list(p, 10 - i);
    sprintf(buf, "%s%d", base_surl, i);
    if(unregister_pfn(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not unregister surl  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* check that the list is finally empty */
  if((p = surlsfromguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
    fail("NULL returned by surlsfromguid");
  }
  if(*p != NULL) {
    fail("there should be no surls initially");
  }
}END_TEST



/** test_surlsfromguid_unknown_guid : check that we get NULL on a unknown
    guid */
START_TEST(test_surlfromguid_unknown_guid) {
  char unknown_guid[CA_MAXGUIDLEN+1];
  char *p;

  helper_make_guid(unknown_guid);

  /* check that we don't get anything initially */
  if((p = surlfromguid (unknown_guid, errbuf, ERRBUFSZ)) != NULL) {
    free(p);
    fail("There should be no surls, since it's not in the db");
  }
  /* and check the error was ENOENT */
  if(errno != ENOENT) {
    printf("ERROR is %d\n", errno);
    fail("Expected ENOENT where guid doesn't exist");
  }
  
}END_TEST

/** test_surlfromguid: check that surl from guid returns an entry.  If
the server is local (cern.ch) then it should be returned */
START_TEST(test_surlfromguid) {
  char *p = NULL;
  char buf[CA_MAXSFNLEN+1];
  char base_surl[CA_MAXSFNLEN+1];
  char *path="foo/test_surlfromguid";
  char *local_server = "foo.cern.ch";
  
  /* check that we don't get anything initially */
  if((p = surlfromguid (file_guid, errbuf, ERRBUFSZ)) != NULL) {
    free(p);
    fail("There should be no surls initially");
  }
  /* and check the error was ENOENT */
  if(errno != ENOENT) {
    snprintf(error_msg, sizeof(error_msg), 
	     "Expected ENOENT where no surls.  Got: %d : %s\n", errno, errbuf);
    fail(error_msg);
  }
  
  /* add an entry */
  helper_make_surl(base_surl, path);
  if(register_pfn (file_guid, base_surl, errbuf, ERRBUFSZ) < 0) {
    sprintf (error_msg, "Could not register surl %s : %s\n",
	     base_surl, strerror (errno));
    fail (error_msg);
  }
  
  if((p = surlfromguid (file_guid, errbuf, ERRBUFSZ)) == NULL) {
    sprintf(error_msg, 
	    "Could not get best surl from guid %s : %s\n",
	    base_surl, strerror (errno));
    fail (error_msg);
  }
  fail_unless(strcmp(p, base_surl) == 0 , "Should be only surl for the guid");
  free(p);
  
  /* now add a local entry - it should be first */
  sprintf(buf, "srm://%s/%s", local_server, path);
  if(register_pfn (file_guid, buf, errbuf, ERRBUFSZ) < 0) {
    sprintf (error_msg, "Could not register surl %s : %s\n",
	     buf, strerror (errno));
    fail (error_msg);
  }
  
  if((p = surlfromguid (file_guid, errbuf, ERRBUFSZ)) == NULL) {
    sprintf(error_msg, 
	    "Could not get best surl from guid %s : %s\n",
	    buf, strerror (errno));
    fail (error_msg);
  }
  fail_unless(strcmp(p, buf) == 0 , "Should be only surl for the guid");
  free(p);
}END_TEST

/** test_lfnsforguid : check that we can add/remove aliases from a guid and
    get the list of entries */
START_TEST(test_lfnsforguid) {
  char **p;
  char **cp;
  char buf[CA_MAXPATHLEN+1];
  int i;

  /* check that the list has one entry - the original lfn */
  if((p = lfnsforguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Should have a lfn for guid  %s : %s : %s\n", 
	      file_guid, file_path, strerror(errno));
      fail(error_msg);
  }
  cp = p;
  fail_unless(strcmp(*cp, file_path) == 0, "Should have orig lfn");
  cp++;
  fail_unless(*cp == NULL, "and only the orig lfn");
  free(*p); free(p);

  /* now add some, and check the size */
  for (i = 1; i < 10 ; ++i) {
    sprintf(buf, "%s%d", file_path, i);
    if(register_alias(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not register alias  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
    if((p = lfnsforguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Could not get lfns for guid  %s : %s\n", 
	      file_guid, strerror(errno));
      fail(error_msg);
    }
    free_list(p, i +1);
  }

  /* and remove them again */
  for (i = 1; i < 10 ; ++i) {
    if((p = lfnsforguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Could not get lfns for guid  %s : %d : %s\n", 
	      file_guid, i, strerror(errno));
      fail(error_msg);
    }
    free_list(p, 11 - i);
    sprintf(buf, "%s%d", file_path, i);
    if(unregister_alias(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not unregister lfn  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* check that the list has only the original lfn at the end */
  if((p = lfnsforguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
      sprintf(error_msg, "Should have a lfn for guid  %s : %s : %s\n", 
	      file_guid, file_path, strerror(errno));
      fail(error_msg);
  }
  cp = p;
  fail_unless(strcmp(*cp, file_path) == 0, "Should have orig lfn");
  cp++;
  fail_unless(*cp == NULL, "and only the orig lfn");
  free(*p); free(p);
}END_TEST

/** test_delete_lfns_master_first : check that we can remove aliases
    from a lfn which we've already deleted */
START_TEST(test_delete_lfns_master_first) {
  char **p;
  char buf[CA_MAXPATHLEN+1];
  int i;

  /* now add some aliases */
  for (i = 1; i < 10 ; ++i) {
    sprintf(buf, "%s%d", file_path, i);
    if(register_alias(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not register alias  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* remove the master */
  if(unregister_alias(file_guid, file_path, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not unregister master lfn  %s : %s\n", 
	      file_path, strerror(errno));
      fail(error_msg);
  }

  /* and remove the aliases again */
  for (i = 1; i < 10 ; ++i) {
    sprintf(buf, "%s%d", file_path, i);
    if(unregister_alias(file_guid, buf, errbuf, ERRBUFSZ) < 0) {
      sprintf(error_msg, "Could not unregister lfn  %s : %s\n", 
	      buf, strerror(errno));
      fail(error_msg);
    }
  }

  /* check that there are no elements for the guid */
  if((p = lfnsforguid(file_guid, errbuf, ERRBUFSZ)) == NULL) {
    if(errno == ENOENT) 
      return;
    sprintf(error_msg, "Can't get lfn for guid with no entries  %s : %s\n", 
	    file_guid, strerror(errno));
    fail(error_msg);
  }
  fail("Should have ENOENT on lfnsforguid() with non-existent guid");
}END_TEST

Suite *add_catalog_tests(Suite *s) { 

  TCase *tc_catalog_env;

  TCase *tc_edg_catalog;
  TCase *tc_lfc_catalog;
  
  /* a test case to check for a given env */
  tc_catalog_env = tcase_create("CatalogFromEnv");
  suite_add_tcase(s, tc_catalog_env);

  tcase_add_test(tc_catalog_env, test_get_cat_type);
  tcase_add_test(tc_catalog_env, test_get_default_catalog);

  /* a test case for EDG catalog checks */
  tc_edg_catalog = tcase_create("EDGCatalogs");
  //suite_add_tcase(s, tc_edg_catalog);
  tcase_add_checked_fixture(tc_edg_catalog, setup_edg_catalog, teardown_common);

  //tcase_add_test(tc_edg_catalog, test_guid_exists);
  tcase_add_test(tc_edg_catalog, test_create_lfn);
  //tcase_add_test(tc_edg_catalog, test_register_before_create);
  tcase_add_test(tc_edg_catalog, test_multiple_register_lfn);

  tcase_add_test(tc_edg_catalog, test_registerpfn);
  //tcase_add_test(tc_edg_catalog, test_registerpfn_exists);
  tcase_add_test(tc_edg_catalog, test_registerpfn_guid_exists);
  tcase_add_test(tc_edg_catalog, test_unregisterpfn);
  tcase_add_test(tc_edg_catalog, test_unregisterpfn_nexist);
  //tcase_add_test(tc_edg_catalog, test_getfilesize);

  tcase_add_test(tc_edg_catalog, test_replica_exists_unknown_guid);
  tcase_add_test(tc_edg_catalog, test_replica_exists);

  tcase_add_test(tc_edg_catalog, test_surlsfromguid_unknown_guid);
  tcase_add_test(tc_edg_catalog, test_surlsfromguid);
  tcase_add_test(tc_edg_catalog, test_surlfromguid_unknown_guid);
  //tcase_add_test(tc_edg_catalog, test_surlfromguid);

  tcase_add_test(tc_edg_catalog, test_lfnsforguid);
  tcase_add_test(tc_edg_catalog, test_delete_lfns_master_first);

  /* LFC Catalog tests */

  tc_lfc_catalog = tcase_create("LFCCatalogs");
  suite_add_tcase(s, tc_lfc_catalog);

  tcase_add_checked_fixture(tc_lfc_catalog, setup_lfc_catalog, teardown_common);

  tcase_add_test(tc_lfc_catalog, test_guid_exists);
  tcase_add_test(tc_lfc_catalog, test_create_lfn);
  tcase_add_test(tc_lfc_catalog, test_register_before_create);
  tcase_add_test(tc_lfc_catalog, test_multiple_register_lfn);

  tcase_add_test(tc_lfc_catalog, test_registerpfn);
  tcase_add_test(tc_lfc_catalog, test_registerpfn_exists);
  tcase_add_test(tc_lfc_catalog, test_registerpfn_guid_exists);
  tcase_add_test(tc_lfc_catalog, test_unregisterpfn);
  tcase_add_test(tc_lfc_catalog, test_unregisterpfn_nexist);
  tcase_add_test(tc_lfc_catalog, test_getfilesize);

  tcase_add_test(tc_lfc_catalog, test_replica_exists_unknown_guid);
  tcase_add_test(tc_lfc_catalog, test_replica_exists);
  tcase_add_test(tc_lfc_catalog, test_surlsfromguid_unknown_guid);
  tcase_add_test(tc_lfc_catalog, test_surlsfromguid);
  tcase_add_test(tc_lfc_catalog, test_surlfromguid_unknown_guid); 
  tcase_add_test(tc_lfc_catalog, test_surlfromguid);

  tcase_add_test(tc_lfc_catalog, test_lfnsforguid);
  tcase_add_test(tc_lfc_catalog, test_delete_lfns_master_first);

  return s; 
}
