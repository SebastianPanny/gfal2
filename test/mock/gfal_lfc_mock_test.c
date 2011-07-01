/*
 * 
 *  convenience function for the mocks or the lfc interface
 * 
 */

#include <cgreen/cgreen.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "gfal_constants.h"
#include "gfal_lfc_mock_test.h"




int lfc_last_err=0;
struct lfc_filestatg* defined_filestatg=NULL;
struct lfc_filestat* defined_filestat=NULL;
struct lfc_filereplica* define_lastfilereplica=NULL;
struct lfc_linkinfo *define_linkinfos=NULL;
int define_numberlinkinfos;
int define_numberreplica;

void define_mock_linkinfos(int number, char** resu){
	int i;
	define_linkinfos= calloc(sizeof(struct lfc_linkinfo), number);
	for(i=0; i < number; ++i){
		g_strlcpy(define_linkinfos[i].path, resu[i], 1024);
	}
	define_numberlinkinfos= number;
	
}

void define_mock_filestatg(mode_t mode, int gid, int uid){
	defined_filestatg= calloc(sizeof(struct lfc_filestatg),1);
	defined_filestatg->filemode = mode;
	defined_filestatg->uid = uid ;
	defined_filestatg->gid= gid ;		
}

void define_mock_filelstat(mode_t mode, int gid, int uid){
	defined_filestat = g_new0(struct lfc_filestat, 1);
	defined_filestat->filemode = mode;
	defined_filestat->uid = uid;
	defined_filestat->gid = gid;
}

void define_mock_filereplica(int n, char** rep_turls){
	define_numberreplica =n;
	define_lastfilereplica = g_new0(struct lfc_filereplica, n);
	int i;
	for(i=0; i< n; ++i)
		strcpy(define_lastfilereplica[i].sfn, rep_turls[i]);
	
 }

int	lfc_mock_statg(const char * lfn, const char * guid, struct lfc_filestatg * f){
	int val = (int) mock(lfn, guid, f);
	if( val == 0){
		if(defined_filestatg)
			memcpy(f, defined_filestatg, sizeof(struct lfc_filestatg));
		return 0;
	}else{
		lfc_last_err = val;
		return -1;
	}
}

int lfc_mock_rename(const char * oldpath, const char* newpath){
	int r =  (int) mock(oldpath, newpath);
	if(r){
		lfc_last_err = r;
		return -1;
	}else
		return 0;
}

int	lfc_mock_starttrans(const char * server, const char * comment){
	int a=  mock(server,comment);
	if(a){
		lfc_last_err = a;
		return -1;
	}	
	return 0;	
}

int lfc_mock_aborttrans(){
	int a=  mock();
	if(a){
		lfc_last_err = a;
		return -1;
	}	
	return 0;		
}

int	lfc_mock_endtrans(){
	int a=  mock();
	if(a){
		lfc_last_err = a;
		return -1;
	}	
	return 0;	
}

int	lfc_mock_lstatg(const char * lfn, struct lfc_filestat * f){
	int val = (int) mock(lfn, f);
	if( val == 0){
		if(defined_filestat)
			memcpy(f, defined_filestat, sizeof(struct lfc_filestat));
		return 0;
	}else{
		lfc_last_err = val;
		return -1;
	}
}

int lfc_mock_access(const char * path, int mode){
	int r =  (int) mock(path, mode);
	if(r){
		lfc_last_err = r;
		return -1;
	}else
		return 0;
}

int lfc_mock_getreplica(const char *path, const char *guid, const char *se, int *nbentries, struct lfc_filereplica **rep_entries){
	int r =  (int) mock(path, guid, se, nbentries, rep_entries);
	if(r){
		lfc_last_err = r;
		return -1;
	}
	*nbentries= define_numberreplica;
	*rep_entries = define_lastfilereplica;
	return 0;	
}

int lfc_mock_getlinks(const char *path, const char *guid, int *nbentries, struct Cns_linkinfo **linkinfos){
	int r =  (int) mock(path, guid,  nbentries, linkinfos);
	if(r){
		lfc_last_err = r;
		return -1;
	}
	*linkinfos= define_linkinfos;
	*nbentries = define_numberlinkinfos;
	return 0;		
}

int lfc_mock_chmod(const char* path, mode_t mode){
	int a=  mock(path, mode);
	if(a){
		lfc_last_err = a;
		return -1;
	}	
	return 0;	
}

int lfc_mock_mkdir(const char* path, const char* guid, mode_t mode){
	int a=  mock(path, guid,  mode);
	if(a){
		lfc_last_err = a;
		return -1;
	}	
	return 0;	
}






