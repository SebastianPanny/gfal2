/*
 *  Doc File for the SRM API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/

#include <glib.h>

/**
	\defgroup srm_group all SRM functions
*/

/**
	\addtogroup srm_group
	@{
*/
/**
 	\note 
		int gfal_turlsfromsurls (gfal_handle, GError** err) :	removed because of redundance with standard srm interface 

;
*/

/**
	\brief init the gfal srm interface for surl -> turls transformation
*/
gfal_handle gfal_init(GError** err);
/**
	\remove a lsit of dir

*/

int gfal_deletesurls (gfal_handle, GList* lsurl, GError** err);
/**
	\brief remove a dir, only one at a time
*/
int gfal_removedir (gfal_handle, gchar* surl, GError** err);

/**
	\brief get a turls list from a surls list
	@param full list of surls that will be converted
	@param err if err=NULL, error report ignored else must *err=NULL and err will be set to the correct error
*/
int gfal_get(gfal_handle, GList* surl_list,  GError** err);

/**
	\state state of the actual request
*/

int gfal_getstatus (gfal_handle, GError** err);
/**
 * 	\brief describe handle state
 *  \return a string description of the state of the handle
 *  \param gfal_handle
 */
char * gfal_getHandleState(gfal_handle);

int gfal_bringonline (gfal_handle, GError** err);

int gfal_prestage (gfal_handle, GError** err);

int gfal_prestagestatus (gfal_handle, GError** err);

int gfal_pin (gfal_handle, GError** err);

int gfal_release (gfal_handle, GError** err);

int gfal_set_xfer_done (gfal_handle, GError** err);

int gfal_set_xfer_running (gfal_handle, GError** err);

int gfal_abortrequest (gfal_handle, GError** err);

int gfal_abortfiles (gfal_handle, GError** err);

int gfal_get_results (gfal_handle, gfal_filestatus **, GError** err);

int gfal_get_ids_setype (gfal_handle, enum se_type *, int *, int **, char **);

int gfal_get_ids (gfal_handle, int *, int **, char **);

int gfal_set_ids (gfal_handle, int, const int *, int, const char *, char *, int);

/**
	\brief release the handle
*/
void gfal_handle_free (gfal_handle);

/**
	@}
*/
