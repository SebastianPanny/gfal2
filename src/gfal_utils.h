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

/**
 * Remove multiple, subsequent characters from a string. For instance:
 *
 * gfal_consolidate_multiple_characters("charrrrrstring, 'r');
 *
 * returns "charstring".
 *
 * The returned string is dynamically allocated, free it!
 *
 * @param s the string to be consolidated. 
 * @param c the character to look for
 * @param start consolidating starts from this index. Before this index, all the characters
 *              are copied.
 *
 * @return the consolidated string, dynamically allocated, or NULL if s was NULL.
 */
char* gfal_consolidate_multiple_characters(const char* s, const char c, const int start);

#endif // #define _GFAL_UTILS_H
