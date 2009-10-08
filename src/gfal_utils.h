/*
 * Copyright (C) 2009 by CERN
 *
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 */

/*
 * @(#)$RCSfile: gfal_utils.h,v $ $Revision: 1.1 $ $Date: 2009/10/08 15:32:39 $ CERN Remi Mollon
 */
#ifndef _GFAL_UTILS_H
#define _GFAL_UTILS_H

/**
 * Count the elements of a NULL-terminated string array.
 *
 * @param a the string array.
 *
 * @return The number of elements in the array, excluding the terminating NULL.
 */
int gfal_count_elements_of_string_array(char** a);

#endif // #define _GFAL_UTILS_H
