/**
   \page faq
 *  Doc File for a FAQ
 * 
 * \author : Devresse Adrien
 *


  
	<h3> 1. Which are the main changes from gfal 1.0 ? </h3>
		- GFal 2.0 is independant of the technologies provided ( plugin system )
		- GFal 2.0 is now designed to be as POSIX as possible, No non-posix like functions will be provided in the future
		- GFal 2.0 provide the getxattr, setxattr, listxattr calls for the plateform's specific calls ( turl resolution, guid resolution, ...)

	<h3> 2. I would make a SRM to TURL resolution with GFAL 2.0 and the gfal_srm_plugin, How to do it ? </h3>
		- char turl[2048];
		- gfal_getxattr("srm://masrul/monfichier", "srm.turl", turl, 2048);

	<h3> 3. I would make a GUID to LFC resolution with GFAL 2.0 and the gfal_lfc_plugin, How to do it ? </h3>
		- char lfn[2048];
		- gfal_getxattr("srm://masrul/monfichier", "lfc.guid", lfn, 2048);

	<h3 > 4. I want a more verbose error report, How to do it ? </h3>
		- \ref gfal_set_verbose( GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE );
		- [...]
		- \ref gfal_posix_check_error(); // called after each gfal_* call

	<h3> 4. I need to create a GFAL 2.0 plugin, How to do it ? </h3>
		- a tutorial will come for more details.

	<h3> 5 . I need to implement a specific non-POSIX call in GFAL, How to do it ? </h3>
		- just register the proper getxattr or setxattr with a specific key.

	<h3> 6. What about the licence ?  : </h3> 
		- The license is still unchanged from gfal 2.0, It is a Apache 2.0 license : \ref apl




*/
