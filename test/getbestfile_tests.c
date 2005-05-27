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
#include <ctype.h>

#include "gfal_api.h"
#include "test_helpers.h"

char errmsg[1024];
int getdomainnm (char *name, int namelen);


void setup_getbestfile() {
  setenviron("LCG_GFAL_VO", "test");
  clearenviron("VO_TEST_DEFAULT_SE");
}

void teardown_getbestfile() {

}

int localdomain_host(char *buf, int buflen, const char *host) {
  char domainname[64];
  getdomainnm (domainname, sizeof(domainname));

  if(strlen(domainname) + strlen(host) + 2 > buflen) {
    return (-1);
  }
  sprintf(buf, "%s.%s", host, domainname);
  return (0);
} 

/* getbestfile_tests : These test the getbestfile method */
START_TEST(test_default_se) {
  char *default_se = "default-se.foo.com";
  char **surls; 
  char *best;
  char *surl1 = "sfn://host1.example.com/file1", *surl2 = "srm://host2.example.com:8443/file3";
  int i = 0;

  setenviron("VO_TEST_DEFAULT_SE", default_se);

  if((surls = calloc(3, sizeof(char*))) == NULL) {
    fail ("Could not allocoate space for surls");
  }
  surls[i++] = strdup(surl1);
  surls[i++] = strdup(surl2);
  surls[i]= (char *)malloc (13 + strlen(default_se));
  sprintf(surls[i++], "srm://%s/file3", default_se);

  best = getbestfile(surls, 3, NULL, 0);
  if(strstr(best, default_se) == NULL) {
    sprintf(errmsg, "Did not return default SE : %s : %s", default_se, best);
    fail(errmsg);
  }
}END_TEST

START_TEST(test_default_se_no_match) {
  char *default_se = "default-se.foo.com";
  char **surls; 
  char *best;
  char *surl1 = "sfn://host1.example.com/file1", *surl2 = "srm://host2.example.com:8443/file3";
  int i = 0;

  setenviron("VO_TEST_DEFAULT_SE", default_se);

  if((surls = calloc(2, sizeof(char*))) == NULL) {
    fail ("Could not allocoate space for surls");
  }
  surls[i++] = strdup(surl1);
  surls[i++] = strdup(surl2);

  /* set the default SE */

  best = getbestfile(surls, 2, NULL, 0);
  if(strstr(best, default_se) != NULL) {
    sprintf(errmsg, "Returned default SE : %s : %s", default_se, best);
    fail(errmsg);
  }

}END_TEST

/** test_localdomain : If a site with a localdomain name is in it, it should
    be selected first */
START_TEST(test_localdomain) {
  char hostbuf[64];
  char **surls; 
  char *best;
  char *surl1 = "sfn://host1.example.com/file1", *surl2 = "srm://host2.example.com:8443/file3";
  int i = 0;

  clearenviron("VO_TEST_DEFAULT_SE");
  if((surls = calloc(3, sizeof(char*))) == NULL) {
    fail ("Could not allocoate space for surls");
  }
  surls[i++] = strdup(surl1);
  surls[i++] = strdup(surl2);

  localdomain_host (hostbuf, 64, "local");
  surls[i]= (char *)malloc (13 + strlen(hostbuf));
  sprintf(surls[i++], "srm://%s/file2", hostbuf);

  /* this should return the localdomain one */
  best = getbestfile(surls, 3, NULL, 0);
  if(strstr(best, hostbuf) == NULL) {
    sprintf(errmsg, "Did not return local domain host : %s : %s", hostbuf, best);
    fail(errmsg);
  }

}END_TEST

Suite *add_getbestfile_tests(Suite *s) { 

  TCase *tc_getbestfile;
  tc_getbestfile = tcase_create("GetBestFile");
  suite_add_tcase(s, tc_getbestfile);
  tcase_add_checked_fixture(tc_getbestfile, setup_getbestfile, teardown_getbestfile);
  tcase_add_test(tc_getbestfile, test_localdomain);
  tcase_add_test(tc_getbestfile, test_default_se);
  tcase_add_test(tc_getbestfile, test_default_se_no_match);
  return s; 
}
