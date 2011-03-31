/*
 *  Doc File for the SRM API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/

/**
	\defgroup srm_group all SRM functions
*/

/**
	\addtogroup srm_group
	@{
*/

gfal_handle gfal_init()

int gfal_deletesurls (gfal_handle, char *, int);

int gfal_removedir (gfal_handle, char *, int);

int gfal_turlsfromsurls (gfal_handle, char *, int);

int gfal_get (gfal_handle, char *, int);

int gfal_getstatus (gfal_handle, char *, int);
/**
 * 	\brief describe handle state
 *  \return a string description of the state of the handle
 *  \param gfal_handle
 */
char* gfal_getHandleState(gfal_handle);

int gfal_bringonline (gfal_handle, char *, int);

int gfal_prestage (gfal_handle, char *, int);

int gfal_prestagestatus (gfal_handle, char *, int);

int gfal_pin (gfal_handle, char *, int);

int gfal_release (gfal_handle, char *, int);

int gfal_set_xfer_done (gfal_handle, char *, int);

int gfal_set_xfer_running (gfal_handle, char *, int);

int gfal_abortrequest (gfal_handle, char *, int);

int gfal_abortfiles (gfal_handle, char *, int);

int gfal_get_results (gfal_handle, gfal_filestatus **);

int gfal_get_ids_setype (gfal_handle, enum se_type *, int *, int **, char **);

int gfal_get_ids (gfal_handle, int *, int **, char **);

int gfal_set_ids (gfal_handle, int, const int *, int, const char *, char *, int);

void gfal_handle_free (gfal_handle);

/**
	@}
*/
