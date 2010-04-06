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

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <check.h>

/* definition of suites */
Suite* add_catalog_tests(Suite *);
Suite* add_getbestfile_tests(Suite *);

int main (int argc, char** argv) { 
  int nf; 
  Suite *s;
  SRunner *sr;
  char* hostname = NULL;
  
  s = suite_create("GFAL suite");
  add_catalog_tests(s);
  add_getbestfile_tests(s); 
  sr = srunner_create(s); 
  
  /* check for LFC_HOST env and fail early */
  if((hostname=getenv("LFC_HOST")) == NULL) {
    fprintf(stderr, "LFC_HOST environment not set.  Please check configuration.\n");
    return EXIT_FAILURE;
  }
  printf("Using LFC catalog host : %s\n", hostname); 

  /* and run the tests */
  srunner_set_fork_status (sr, CK_FORK);
  srunner_run_all (sr, CK_NORMAL); 
  nf = srunner_ntests_failed (sr); 
  srunner_free (sr); 
  return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
   
