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


char* gfal_add_strings(const char* s1, const char* s2)
{
    char* ret = 0;
    unsigned int len_s1 = 0;

    assert(s1);
    assert(s2);
   
    if (!s1 || !s2) {
        return 0;
    }

    len_s1 = strlen(s1);
    ret = malloc(len_s1 + strlen(s2) + 1);
    assert(ret);
   
    if (ret) {
        strcpy(ret, s1);
        strcat(ret + len_s1, s2);
    }

    return ret;
}

char* gfal_strip_string(const char* str, const char chr)
{
    char *res = 0;

    assert(str);

    if (str) {
        char *pos = strrchr(str, chr);
        int size = -1;

        if (pos) {
            /* +1: to include the last character as well */
            size = pos - str + 1; 
        } else {
            size = strlen(str);
        }

        res = (char*) malloc(size + 1);
        strncpy(res, str, size);
        res[size] = 0;
    }

    return res;
}
