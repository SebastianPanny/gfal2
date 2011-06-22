#pragma once
/*

*	auto-generated header file for file test/mock/gfal_lfc_mock_test.c 
 
*/

#include <lfc_api.h>

extern int lfc_last_err;
extern struct lfc_filestatg* defined_filestatg;
extern struct lfc_filestat* defined_filestat;
extern struct lfc_filereplica* define_lastfilereplica;
extern struct lfc_linkinfo *define_linkinfos;
extern int define_numberlinkinfos;
extern int define_numberreplica;

int	lfc_mock_statg(const char * lfn, const char * guid, struct lfc_filestatg * f);

int	lfc_mock_lstatg(const char * lfn, struct lfc_filestat * f);

int lfc_mock_rename(const char * oldpath, const char* newpath);

int lfc_mock_access(const char* path, int mode);


int lfc_mock_getreplica(const char *path, const char *guid, const char *se, int *nbentries, struct lfc_filereplica **rep_entries);

int lfc_mock_getlinks(const char *path, const char *guid, int *nbentries, struct Cns_linkinfo **linkinfos);

void lfc_next_valid_statg_values(const char* lfn, const char* guid, struct lfc_filestatg * f);
void lfc_statg_all_invalid();

