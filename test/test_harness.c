/*
 * Copyright (C) 2004 by CERN
 * All rights reserved
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <check.h>

/* definition of suites */
Suite* add_catalog_tests(Suite *);

int main (int argc, char** argv) { 
  int nf; 
  Suite *s = suite_create("GFAL suite");
  add_catalog_tests(s); 

  SRunner *sr = srunner_create(s); 
  char* hostname = NULL;
  char hostname_env[128];
  
  /* check for LFC_HOST env and fail early */
  if((hostname=getenv("LFC_HOST")) == NULL) {
    fprintf(stderr, "LFC_HOST environment not set.  Please check configuration.\n");
    return EXIT_FAILURE;
  }
  printf("Running tests for catalog host : %s\n", hostname); 

  /* and run the tests */
  srunner_set_fork_status (sr, CK_NOFORK);
  srunner_run_all (sr, CK_NORMAL); 
  nf = srunner_ntests_failed (sr); 
  srunner_free (sr); 
  return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
   
