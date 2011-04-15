/**
	\mainpage Gfal Documentation
	
	\author Devresse Adrien ( adev88@gmail.com )
	
	<h1> SUMMARY : </h1>
	
	<h2>I.Report of the Scheduled changes : </h2>
	- The list of changes from 1.X to 2.X are resumed \ref api_change 
	
	<h2>II. How to compile </h2>
	- Compile :
		- follow gfal hello world tutorial to have an etics installation
		- Install scons ( http://www.scons.org, python script )
		- checkout the source from the svn : git svn clone http://svnweb.cern.ch/guest/lcgutil/gfal
		- "cd CERNgfal"
		- "scons "
		- configure env vars ETICS_WORKSPACE if asked.
	- Compile tests :
		- Install python >=2.4
		- Install scons 
		- execute "scons testing" in the CERNgfal dir
	- Clean builds :
		- scons -c
	
	

*/
