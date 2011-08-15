/**
	\mainpage Gfal Documentation
	
	\author Devresse Adrien ( adrien.devresse@cern.ch )
	
	 <h1> Main API description here :  </h1> <b> <center> \ref posix_group</b> </center>
		
	<h1>I.Report of the Scheduled changes : </h1>
	- Changes between 1.X and 2.X are resumed \ref api_change 
	
	<h1>II. How to compile locally</h1>
	- Compile :
		- follow tutorial to have an etics installation and a gfal build on your pc
		- Install scons ( http://www.scons.org, python script )
		- " git svn clone  http://svnweb.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2 "
		- " cd gfal2 "
		- "scons -j 8"
	- Compile tests :
		- Install python >=2.4
		- Install scons 
		- execute "scons test" in the CERNgfal dir
	- Clean builds :
		- scons -c
	
	

*/
