/**
  \page api_change
  
  
	<h1> API CHANGE 2.0 </h1> \n
	List of the scheduled changes for the GFal 2.0 API
	\warning  this is not a final choice

	<h2> I. OFFICIAL API : </h2>
	
	- Gfal POSIX lib API:
		- the Common functions API : \ref common_group \n
		- the POSIX style API : \ref posix_group \n
		\n
	- Gfal REQUEST lib  API : 
		- the SRM style API : \ref srm_group \n
		- <b> internal API, use at your own risks </b> LFC_API : \ref lfc_low_group
		- <b> internal API, use at your own risks </b> SFN_API : \ref sfn_low_group 
		- <b> internal API, use at your own risks </b> internal functions of gfal : \ref internal_group

	<h2> II. ANOTHER DOCS : </h2>
		- More informations about the GError system : http://developer.gnome.org/glib/stable/glib-Error-Reporting.html
		- More informations about the use of GList : http://developer.gnome.org/glib/stable/glib-Doubly-Linked-Lists.html
		- More informations about the Gobject system : http://developer.gnome.org/gobject/stable/
*/







/**
 * \defgroup internal_group internal functions
 * */
 
 /**
  * \addtogroup internal_group
  * @{
  */
  
struct gfal_handle_ {
	// INPUTS

	int							generatesurls;
	char *						relative_path;
	int							nbfiles;
	char **						surls;
	char *						endpoint;
	int							oflag;
	GFAL_LONG64 *				filesizes;
	enum se_type				defaultsetype;
	enum se_type				setype;
	int							no_bdii_check;
	int							timeout;
	char **						protocols;

	// Only used with SRMv2
	char *						srmv2_spacetokendesc;
	int							srmv2_desiredpintime;
	int							srmv2_lslevels;
	int							srmv2_lsoffset;
	int							srmv2_lscount;

	// OUTPUTS

	// For Classic SEs
	struct sfn_filestatus *		sfn_statuses;

	// For SRMv1 SEs
	int							srm_reqid;
	struct srm_filestatus *		srm_statuses;
	struct srm_mdfilestatus *	srm_mdstatuses;

	// For SRMv2.2 SEs
	char *						srmv2_token;
	struct srmv2_filestatus *	srmv2_statuses;
	struct srmv2_pinfilestatus *srmv2_pinstatuses;
	struct srmv2_mdfilestatus *	srmv2_mdstatuses;

	int free_endpoint;

	// Common results
	int returncode; // size of the new results
	int results_size; // size of the old results
	gfal_filestatus *results;
};

typedef struct gfal_handle_  *gfal_handle;
  
char * gfal_test__gfal_parseturl();

static char * endpointfromsurl (const char *surl, char *errbuf, int errbufsz, int _prefixing_on);


  //void gfal_errmsg (char *, int, const char *, int);
char *gfal_get_userdn (char *errbuf, int errbufsz);
char *gfal_get_vo (char *errbuf, int errbufsz);
int gfal_get_fqan (char ***fqan, char *errbuf, int errbufsz);
int gfal_is_nobdii ();
int gfal_is_purifydisabled ();
int gfal_register_file (const char *, const char *, const char *, mode_t, GFAL_LONG64, int, char *, int);
void gfal_spacemd_free (int,gfal_spacemd *);
int gfal_guidsforpfns (int, const char **, int, char ***, int **, char *, int);
char *gfal_guidforpfn (const char *, char *, int);
char **gfal_get_aliases (const char *, const char *, char *, int);
int gfal_unregister_pfns (int, const char **, const char **, int **, char *, int);
char **gfal_get_replicas (const char *, const char *, char *, int);
char *gfal_get_hostname (const char *, char *, int);


/******************** gfal_file.c ********************/

gfal_file gfal_file_new (const char *, const char *, int, char *, int);
int gfal_file_free (gfal_file);
const char *gfal_file_get_catalog_name (gfal_file);
const char *gfal_file_get_replica (gfal_file);
int gfal_file_get_replica_errcode (gfal_file);
const char *gfal_file_get_replica_errmsg (gfal_file);
int gfal_file_set_replica_error (gfal_file, int, const char *);
int gfal_file_set_turl_error (gfal_file, int, const char *);
int gfal_file_next_replica (gfal_file);
char *gfal_generate_lfn (char *, int);
char *gfal_generate_guid (char *, int);


/******************** mds_ifce.c ********************/

int gfal_get_bdii (char *, int, int *, char *, int);
int gfal_get_cat_type(char **);
int gfal_get_ce_ap (const char *, char **, char *, int);
int gfal_get_lfc_endpoint (char **, char *, int);
int gfal_get_rls_endpoints (char **, char **, char *, int);
int gfal_get_storage_path (const char *, const char *, char **, char **, char *, int);
int gfal_get_seap_info (const char *, char ***, int **, char *, int);
int gfal_get_se_types_and_endpoints (const char *, char ***, char ***, char *, int);




/******************** rmc_ifce.c ********************/

char *rmc_guidfromlfn (const char *, char *, int);
char **rmc_lfnsforguid (const char *, char *, int);
int rmc_register_alias (const char *, const char *, char *, int);
int rmc_unregister_alias (const char *, const char *, char *, int);


/******************** gridftp_ifce.c ********************/

int gridftp_delete (char *, char *, int, int);
int gridftp_ls (char *, int *, char ***, struct stat64 **, char *, int, int);



/**
 * @}
 * */
 
 
 
