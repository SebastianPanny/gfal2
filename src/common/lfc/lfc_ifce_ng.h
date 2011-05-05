#pragma once
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
  * 
  @file lfc_ifce_ng.h
  @brief internal header of the lfc catalog module
  @author Adrien Devresse
  @version 0.0.1
  @date 02/05/2011
 */
 
#define GFAL_MAX_LFCHOST_LEN 1024

#include <lfc_api.h>

#include "gfal_prototypes.h"
#include "gfal_types.h"





struct lfc_ops {
	char* lfc_endpoint;
	
	int	*serrno;
	char	*(*sstrerror)(int);
	int	(*addreplica)(const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *);
	int	(*creatg)(const char *, const char *, mode_t);
	int	(*delreplica)(const char *, struct lfc_fileid *, const char *);
	int	(*aborttrans)();
	int	(*endtrans)();
	int	(*getpath)(char *, u_signed64, char *);
	int (*getlinks)(const char *, const char *, int *, struct lfc_linkinfo **);
	int (*getreplica)(const char *, const char *, const char *, int *, struct lfc_filereplica **);
	int	(*lstat)(const char *, struct lfc_filestat *);
	int	(*mkdirg)(const char *, const char *, mode_t);
	int	(*seterrbuf)(char *, int);
	int	(*setfsizeg)(const char *, u_signed64, const char *, char *);
	int	(*setfsize)(const char *, struct lfc_fileid *, u_signed64);
	int	(*starttrans)(const char *, const char *);
	int	(*statg)(const char *, const char *, struct lfc_filestatg *);
	int	(*statr)(const char *, struct lfc_filestatg *);
	int	(*symlink)(const char *, const char *);
	int	(*unlink)(const char *);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*rename)(const char *, const char *);
	lfc_DIR *(*opendirg)(const char *, const char *);
	int	(*rmdir)(const char *);
	int (*startsess) (char *, char *); 
	int (*endsess) ();
};

char* gfal_setup_lfchost(gfal_handle handle, GError ** err);


struct lfc_ops* gfal_load_lfc(const char* name, GError** err);


char* gfal_convert_guid_to_lfn(catalog_handle handle, char* guid, GError ** err);

