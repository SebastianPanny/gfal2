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
 
 
 /**
  * @brief file for the storage access part of gfal
  * @author : Devresse Adrien
  * @version 0.0.1
  * @date 12/04/2011
  * */

#include "gfal_common_storage.h"



struct dir_info *
alloc_di (DIR *dir)
{
    int i;

    for (i = 0; i < GFAL_OPEN_MAX; i++) {
        if (di_array[i] == NULL) {
            if ((di_array[i] = (struct dir_info *) calloc (1, sizeof(struct dir_info))) == NULL)
                return (NULL);
            di_array[i]->dir = dir;
            return (di_array[i]);
        }
    }
    errno = EMFILE;
    return (NULL);
}

struct xfer_info *
alloc_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd] == NULL)
        return (xi_array[fd] = (struct xfer_info *) calloc (1, sizeof(struct xfer_info)));
    errno = EBADF;
    return (NULL);
}

struct dir_info *
find_di (DIR *dir)
{
    int i;

    for (i = 0; i < GFAL_OPEN_MAX; i++) {
        if (di_array[i] && di_array[i]->dir == dir)
            return (di_array[i]);
    }
    errno = EBADF;
    return (NULL);
}

struct xfer_info *
find_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd])
        return (xi_array[fd]);
    errno = EBADF;
    return (NULL);
}

void
free_di (struct dir_info *di)
{
    free (di);
    di = NULL;
}

int
free_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd]) {
        if (xi_array[fd]->gfile)
            gfal_file_free (xi_array[fd]->gfile);
        free (xi_array[fd]);
        xi_array[fd] = NULL;
    }
    return (0);
}



