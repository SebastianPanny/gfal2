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
