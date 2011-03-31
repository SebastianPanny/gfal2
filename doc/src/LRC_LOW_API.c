/*
 *  Doc File for the low level LRC API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/



/******************** lrc_ifce.c ********************/

/**
 * \groupdef lrc_low_group  lrc low level functions 
 * 
 * */
 
 /**
  * \addtogroup lrc_low_group
  * @{
	*/

int lrc_deletepfn (const char *, const char *, char *, int);
int lrc_deletesurl (const char *, char *, int);
char *lrc_get_catalog_endpoint(char *, int);
char *lrc_guidforpfn (const char *, char *, int);
int lrc_guid_exists (const char *, char *, int);
int lrc_register_pfn (const char *, const char *, char *, int);
int lrc_replica_exists(const char* ,char*, int);
int lrc_setfilesize (const char *, GFAL_LONG64, char *, int);
int lrc_unregister_pfn (const char *, const char *, char *, int);
char **lrc_surlsfromguid (const char *, char *, int);
int lrc_fillsurls (gfal_file);


/**
 * @{
*/
