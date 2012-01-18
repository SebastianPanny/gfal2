"""
 Arguments Utils, add conveniences functions for arguments management in SCons
"""

#
# __COPYRIGHT__
#
# This file is part of scons-addons.
#
# Scons-addons is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Scons-addons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with scons-addons; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#


import SCons.Environment
import SCons
import SCons.Platform


"""
 Simple way to get generic parameter from command line
"""
def Parameter(name, default_value):
	return SCons.Script.ARGUMENTS.get(name, default_value)


"""
 Simple way to get boolean parameter from command line 
"""
def Parameter_bool(name, default_value):
	if(isinstance(default_value, bool) is False):
		raise Exception(" This parameter is not a boolean" + str(default_value))
	
	def_bool = (default_value is True) and "yes" or "no"
	if SCons.Script.ARGUMENTS.get(name, def_bool) == "yes":
		 return True
	return False
