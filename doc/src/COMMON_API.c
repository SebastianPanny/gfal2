/*
 *  Doc File
 * 
 * 	author : Devresse Adrien
 * 
*/

/**
	\defgroup common_group
	
*/

/**
 * \addtogroup common_group all common functions
 * @{
*/

/**
 * \brief return a string of the current gfal_version
 *  define at the compilation time
 */
const char *gfal_version ();
/**
 * \brief set the verbose global level
 * \param values : { \ref GFAL_VERBOSE_QUIET, \ref GFAL_VERBOSE_NORMAL , \ref GFAL_VERBOSE_VERBOSE , \ref GFAL_VERBOSE_DEBUG  }
 */
int gfal_set_verbose (int);
/**
 * \brief set the vo used by gfal
 */
int gfal_set_vo (const char *vo);
/**
 *  \brief define if bdii System is ignored or not
 *  \param : 0 for False else true
 * */
void gfal_set_nobdii (int);
/**
 * \brief change the default timeout 
 */
void gfal_set_timeout_connect (int);
/**
 * \brief get the current timeout
 * default value : 60
 */
int gfal_get_timeout_connect ();
/**
 * set the value for the send/receive timeout 
 */
void gfal_set_timeout_sendreceive (int);
/**
 *  get the value for the send/receive timeout 
 *  default value : 0
 */
int gfal_get_timeout_sendreceive ();
/**
 * set the timeout for the bdii queries
 */
void gfal_set_timeout_bdii (int);
/**
 * get the current the timeout for the bdii queries
 *  defautl value : 60
 */
int gfal_get_timeout_bdii ();
/**
 * 	set the timeout for the SRM queries
 * */
void gfal_set_timeout_srm (int);
/**
 * 	get the timeout for the SRM queries
 *  default value : 3600
 * */
int gfal_get_timeout_srm ();
const char *gfal_get_checksum_algname (enum gfal_cksm_type);

enum gfal_cksm_type gfal_parse_checksum_algname (const char *);

/**
 * @}
 * */
