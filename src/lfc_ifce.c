/*
 * Copyright (C) 2004 by CERN
 */

/*
 * @(#)$RCSfile: lfc_ifce.c,v $ $Revision: 1.16 $ $Date: 2005/01/17 14:39:42 $ CERN James Casey
 */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "lfc_api.h"
#include "gfal_api.h"
#include "serrno.h"

#define ALLOC_BLOCK_SIZE 16 /* the block size to allocate new pointers in */
char *lfc_host = NULL;

static  char lfc_env[64];

/** extract a hostname from a SURL.  We search for "://" to get the start of
    the hostname.  Then we keep going to the next slash, colon or end of the
    SURL. */
char *
get_hostname(const char *path, char *errbuf, int errbufsz) {
  char *start;
  char *cp;
  char *result;
  char c;
  char sav_path[CA_MAXSFNLEN+1];

  strcpy(sav_path, path);

  start = strchr(sav_path, ':');
  if ( start == NULL || *(start+1) != '/' || *(start+2) != '/') {
    gfal_errmsg(errbuf, errbufsz, "Path too long");
    errno = EINVAL;
    return (NULL);
  }
  /* point start to beginning of hostname */
  start += 3;
  for(cp = start; *cp != '\0' && *cp != ':' && *cp != '/'; cp++)
    ;
  c = *cp;
  *cp = '\0';
  if((result = strdup(start)) == NULL) {
    return (NULL);
  }
  *cp = c;
  return result;
}

static int 
lfc_init (char *errbuf, int errbufsz) {
  if (lfc_host == NULL) {
    if((lfc_host = getenv("LFC_HOST")) == NULL &&
       get_lfc_host(&lfc_host) < 0) {
      gfal_errmsg(errbuf, errbufsz, "LFC host not found");
      errno = EINVAL;
      return (-1);
    }
    if( 10 + strlen(lfc_host) > 64) {
      gfal_errmsg(errbuf, errbufsz, "Host too long") ;
      errno = EINVAL;
      return (-1);
    }
    sprintf(lfc_env, "LFC_HOST=%s", lfc_host);
    if(putenv(lfc_env) < 0) {
      return (-1);
    }
  }
  lfc_seterrbuf(errbuf, errbufsz);
  return (0);
}

int lfc_replica_exists(const char *guid, char *errbuf, int errbufsz) {
  lfc_list list;
  struct lfc_filereplica* rp;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);
  
  if((rp = lfc_listreplica(NULL, guid, CNS_LIST_BEGIN, &list)) == NULL) {
    (void) lfc_listreplica(NULL, guid, CNS_LIST_END, &list);
    return (0);
  } else { 
    (void) lfc_listreplica(NULL, guid, CNS_LIST_END, &list);
    return (1);
  }
}

int
lfc_getfilesizeg(const char *guid, GFAL_LONG64 *sizep, char *errbuf, int errbufsz)
{
  struct lfc_filestatg statg;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);

  if(lfc_statg(NULL, guid, &statg) < 0) {
    if (serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  
  *sizep = (u_signed64) statg.filesize;
  return (0);
}

/** lfc_guidforpfn : Get the guid for a replica.  If the replica does not
    exist, fail with ENOENT */
char *
lfc_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
  char *p;
  struct lfc_filestatg statg;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (NULL);

  if(lfc_statr(pfn, &statg) < 0) {
    if(serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (NULL);
  }
  if((p = strdup(statg.guid)) == NULL) {
    return (NULL);
  }
  return (p);
}

int
lfc_guid_exists (const char *guid, char *errbuf, int errbufsz)
{
  struct lfc_filestatg statg;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);

  if(lfc_statg(NULL, guid, &statg) < 0) {
    if(serrno == ENOENT) 
	return (0);
    if (serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  return (1);
}

int
lfc_register_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
  char *hostname;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);
  
  if((hostname = get_hostname(pfn, errbuf, errbufsz)) == NULL) {
    return (-1);
  }

  if(lfc_addreplica(guid, NULL, hostname, pfn) < 0) {
    if (serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    free(hostname);
    return (-1);
  }
  free(hostname);
  return (0);
}

char **
lfc_surlsfromguid (const char *guid, char *errbuf, int errbufsz)
{
  lfc_list list;
  struct lfc_filereplica* rp;
  int flags;
  char **p, **pp;
  size_t size = ALLOC_BLOCK_SIZE;
  size_t i = 0;
  
  if(lfc_init(errbuf, errbufsz) < 0)
    return (NULL);

  /* allocate some memory for the pointers */
  if((p = (char**)calloc(size, sizeof(char*))) == NULL) {
    return (NULL);
  }

  flags = CNS_LIST_BEGIN;
  while((rp = lfc_listreplica(NULL, guid, flags, &list)) != NULL) {
    if(flags == CNS_LIST_BEGIN) 
      flags = CNS_LIST_CONTINUE;
    
    if((p[i++] = strdup(rp->sfn)) == NULL) {
      (void) lfc_listreplica(NULL, guid, CNS_LIST_END, &list);
      free(p);
      return (NULL);
    }
    
    if(i >= size) {
      size += ALLOC_BLOCK_SIZE;
      if((pp = (char**)realloc(p, size * sizeof(char*))) == NULL) {
	(void) lfc_listreplica(NULL, guid, CNS_LIST_END, &list);
	free(p);
	return (NULL);
      }
      p = pp;
    }
  } 
  (void) lfc_listreplica(NULL, guid, CNS_LIST_END, &list);

  /* no results - return NULL */
  if(i == 0) {
    free (p);
    return (NULL);
  }
  
  p[i++]='\0';
  /* and trim */
  if((pp = (char**)realloc(p, i * sizeof(char*))) == NULL) {
    free(p);
    return (NULL);
  }
  
  return (pp);
}

char *
lfc_surlfromguid (const char *guid, char *errbuf, int errbufsz)
{
  char **surls;
  char **cp;
  char *result;
  
  if(lfc_init(errbuf, errbufsz) < 0)
    return (NULL);

  if((surls = lfc_surlsfromguid(guid, errbuf, errbufsz)) == NULL) {
    errno = ENOENT;
    return (NULL);
  }
  result = getbestfile(surls, (sizeof(surls)/sizeof(char*)));

  for(cp = surls; *cp != NULL; ++cp) {
    if(*cp != result) {
      free (*cp);
    }
  }
  free (surls);
  return (result);
}

/** lfc_unregister_pfn : We unregister a pfn from a guid, but only if it a
    replica for that guid */
int
lfc_unregister_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);
  
  if(lfc_delreplica(guid, NULL, pfn) < 0) {
    if(serrno == ENOENT) {
      return (0);
    }
    if(serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  return (0);
}

char *
lfc_guidfromlfn (const char *lfn, char *errbuf, int errbufsz)
{
  struct lfc_filestatg statg;
  char *p;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (NULL);

  if(lfc_statg(lfn, NULL, &statg) < 0) {
    if(serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (NULL);
  }
  if((p = strdup(statg.guid)) == NULL)
    return (NULL);
  return (p);
}

char **
lfc_lfnsforguid (const char *guid, char *errbuf, int errbufsz)
{
  lfc_list list;
  struct lfc_linkinfo* lp;
  int flags;
  char **p, **pp;
  size_t size = ALLOC_BLOCK_SIZE;
  size_t i = 0;
  
  if(lfc_init(errbuf, errbufsz) < 0)
    return (NULL);

  /* allocate some memory for the pointers */
  if((p = (char**)calloc(size, sizeof(char*))) == NULL) {
    return (NULL);
  }

  flags = CNS_LIST_BEGIN;
  while((lp = lfc_listlinks(NULL, guid, flags, &list)) != NULL) {
    if(flags == CNS_LIST_BEGIN) 
      flags = CNS_LIST_CONTINUE;
    
    if((p[i++] = strdup(lp->path)) == NULL) {
      (void) lfc_listlinks(NULL, guid, CNS_LIST_END, &list);
      free (p);
      return (NULL);
    }
    
    if(i >= size) {
      size += ALLOC_BLOCK_SIZE;
      if((pp = (char**)realloc(p, size * sizeof(char*))) == NULL) {
	(void) lfc_listlinks(NULL, guid, CNS_LIST_END, &list);
	free (p);
	return (NULL);
      }
      p = pp;
    }
  } 
  (void) lfc_listlinks(NULL, guid, CNS_LIST_END, &list);
  /* no results */
  if( i== 0) {
    errno = ENOENT;
    free (p);
    return (NULL);
  }

  p[i++] = '\0';
  if((pp = (char**)realloc(p, i * sizeof(char*))) == NULL) {
    free (p);
    return (NULL);
  }

  return (pp);
}

int
lfc_create_alias (const char *guid, const char *lfn, GFAL_LONG64 size, char *errbuf, int errbufsz)
{
  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);

  lfc_starttrans();
  if(lfc_creatg(lfn, guid, 0775) < 0) {
    if(serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  if(lfc_setfsizeg(guid, size, NULL, NULL) < 0) {
    if(serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  lfc_endtrans();
  return (0);
}

int
lfc_register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
  struct lfc_filestatg statg;
  char master_lfn[CA_MAXPATHLEN+1];
  
  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);

  lfc_starttrans();
  if(lfc_statg(NULL, guid, &statg) < 0) {
    if(serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  /* now we do a getpath() to get the master lfn */
  if (lfc_getpath(lfc_host, statg.fileid, master_lfn) <0 ) {
    if (serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }

  /* and finally register */
  if(lfc_symlink(master_lfn, lfn) < 0) {
    if (serrno < 1000)
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  lfc_endtrans();
  return (0);
}

int 
lfc_unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
  struct lfc_filestatg statg;
  struct lfc_filestat stat;

  if(lfc_init(errbuf, errbufsz) < 0)
    return (-1);

  lfc_starttrans();
  /*  In the case of the master lfn being unlinked already, statg will
      return ENOENT.  We then check lstat in case it's a hanging link ?  */
  if(lfc_statg(lfn, guid, &statg) < 0 ) {
    if (serrno == ENOENT) {
      if(lfc_lstat(lfn, &stat) < 0 ) {
	if(serrno < 1000 ) 
	  errno = serrno;
	else {
	  gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
	  errno = ECOMM;
	}
	return (-1);
      } else {
	/* all ok, continue */
      }
    } else {
      if(serrno < 1000) 
	errno = serrno;
      else {
	gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
	errno = ECOMM;
      }
      return (-1);
    }
  }

  /* lfn maps to the guid - unlink it */
  if(lfc_unlink(lfn) < 0) {
    if(serrno < 1000) 
      errno = serrno;
    else {
      gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
      errno = ECOMM;
    }
    return (-1);
  }
  lfc_endtrans();
  return (0);
}

int 
lfc_mkdirp(const char *path, mode_t mode, char *errbuf, int errbufsz)
{
  int c;
  char *lastslash = NULL;
  char *p;
  char *p1;
  char *q;
  char sav_path[CA_MAXPATHLEN+1];
  struct lfc_filestatg statbuf;
  uuid_t uuid;
  char uuid_buf[CA_MAXGUIDLEN+1];

  if (strlen (path) >= sizeof(sav_path)) {
    gfal_errmsg(errbuf, errbufsz, "Path too long");
    errno = EINVAL;
    return (-1);
  }
  strcpy (sav_path, path);
  p1 = strchr (sav_path, '/');
  p = strrchr (sav_path, '/');
  while (p > p1) {
    if (lastslash == NULL) lastslash = p;
    *p = '\0';
    c = lfc_statg (sav_path, NULL, &statbuf);
    if (c == 0) {
      *p = '/';
      break;
    }
    if (serrno != ENOENT) {
      if(serrno < 1000) 
	errno = serrno;
      else {
	gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
	errno = ECOMM;
      }
      return (c);
    }
    q = strrchr (sav_path, '/');
    *p = '/';
    p = q;
  }
  c = 0;
  while (c == 0 && (p = strchr (p + 1, '/')) && p <= lastslash) {
    *p = '\0';
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_buf);
    c = lfc_mkdirg (sav_path, uuid_buf, mode);
    if(c != 0) {
      if (serrno < 1000)
	errno = serrno;
      else {
	gfal_errmsg(errbuf, errbufsz, sstrerror(serrno));
	errno = ECOMM;
      }
    }
    *p = '/';
  }
  return (c);
}
  
