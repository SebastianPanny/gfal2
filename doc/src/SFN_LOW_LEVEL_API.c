/*
 *  Doc File for the low level sfn API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/

/**
 * \defgroup sfn_low_group sfn low level functions
 * */
 
 /**
  * \addtogroup sfn_low_group 
  * @{
*/

/******************** sfn_ifce.c ********************/

int sfn_deletesurls (int, const char **, struct sfn_filestatus **, char *, int, int);
int sfn_getfilemd (int, const char **, struct srmv2_mdfilestatus **, char *, int, int);
int sfn_turlsfromsurls (int, const char **, char **, struct sfn_filestatus **, char *, int);

/**
	@}
*/
