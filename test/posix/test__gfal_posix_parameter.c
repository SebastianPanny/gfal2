

/* unit test for the set/get parameters */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <posix/gfal_posix_api.h>




void test_posix_set_get_parameter(){
    char buff[2048];
    buff[0]='\0';
  
    int ret = gfal_set_parameter(NULL, "no_bdii", "True", 255); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return");
    
    ret = gfal_get_parameter(NULL, "no_bdii", buff, 2048); // verify the variable status 
    
    assert_true_with_message(ret == 0 && (strcmp(buff, "True") == 0), " must be the value set before");
    
    ret = gfal_set_parameter(NULL, "no_bdii", "False", 255); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return 2");
    
    ret = gfal_get_parameter(NULL, "no_bdii", buff, 2048); // verify the variable status 
    
    assert_true_with_message(ret == 0 && (strcmp(buff, "False") == 0), " must be the value set before 2");

    
  
  
}