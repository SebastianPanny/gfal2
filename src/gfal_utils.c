/*
 * Copyright (C) 2009 by CERN
 *
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 */

/*
 * @(#)$RCSfile: gfal_utils.c,v $ $Revision: 1.1 $ $Date: 2009/10/08 15:32:39 $ CERN Remi Mollon
 */
#include "gfal_utils.h"
#include <assert.h>

int gfal_count_elements_of_string_array(char** a)
{
    int ret = -1;
    assert(a);
    for (ret = 0; a[ret] != 0; ++ret) ;
    return ret;
}


char* gfal_consolidate_multiple_characters(const char* s, const char c, const int start)
{
    char *tmp = 0;
    char *ret = 0;
    int i = 0;
    int tmp_i = 0;

    if (s == 0) {
        return 0;
    }    

    tmp = (char*) malloc (strlen(s) + 1);

    for (; s[i] != 0; ++i) {
        // Copy the characters unless we find c. If the index + 1 also stores c, do not copy,
        if (i < start || s[i] != c || s[i + 1] != c) {
            tmp[tmp_i] = s[i];
            ++tmp_i;
        } 
    } 
    
    tmp[tmp_i] = 0;
    // strdup the string, to shrink to the real size
    ret = strdup(tmp);    
    free(tmp);
    return ret;
}

