/**
	\mainpage Gfal Documentation
	
	\author Devresse Adrien ( adev88@gmail.com )
	
	 <h1> Main API description here :  </h1> <b> <center> \ref posix_group</b> </center>
		
	<h1>I.Report of the Scheduled changes : </h1>
	- Changes between 1.X and 2.X are resumed \ref api_change 
	
	<h1>II. How to compile locally</h1>
	- Compile :
		- follow tutorial to have an etics installation and a gfal build on your pc
		- Install scons ( http://www.scons.org, python script )
		- checkout the source from the svn : git svn clone http://svnweb.cern.ch/guest/lcgutil/gfal
		- "cd CERNgfal"
		- "scons -j 8"
		- configure env vars ETICS_WORKSPACE if asked.
	- Compile tests :
		- Install python >=2.4
		- Install scons 
		- execute "scons test" in the CERNgfal dir
	- Clean builds :
		- scons -c
	
	

*/
