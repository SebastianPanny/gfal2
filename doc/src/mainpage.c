/**
	\mainpage Gfal Documentation
	
	\author Devresse Adrien ( adrien.devresse@cern.ch )
	
		

	<h2>I.FAQ  : </h2>
	- \ref faq

	<h2> II API : </h2>
	
	- Gfal POSIX lib API:
		- the POSIX style API : \ref posix_group \n
		- the Error management API : \ref gfal_posix_check_error

	- Gfal internal API to construct plugin ( not stable currently ) :
		- \ref _gfal_catalog_interface


	<h1>III.Report of the Scheduled changes : </h1>
	- Changes between 1.X and 2.X are resumed \ref api_change 	
	
	<h1>IV. How to compile locally</h1>
	- Compile :
		- follow tutorial to have an etics installation and a gfal build on your pc
		- Install scons ( http://www.scons.org, python script )
		- " git svn clone  http://svnweb.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2 "
		- " cd gfal2 "
		- "scons -j 8"
	- Compile tests :
		- Install cgreen, unit test framework
		- Install scons 
		- execute "scons test -j 8" in the CERNgfal dir
	- Clean builds :
		- scons -c
	


*/
