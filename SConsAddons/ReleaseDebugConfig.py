"""
 ReleaseDebugConfig Utils, add conveniences functions for debug/release flag in Scons
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

debug_c_flags = "-g"
profile_c_flags = "-pg"
release_c_flags = "-O3"
waning_c_flags = "-Wall"

def ConfigureBuildTarget(env):
	if SCons.Script.ARGUMENTS.get('debug','0') =='yes':
		env.Append(CFLAGS=debug_c_flags)
	if SCons.Script.ARGUMENTS.get('profile','0') =='yes':
		env.Append(CFLAGS=profile_c_flags)
	if (SCons.Script.ARGUMENTS.get('production','0') =='yes' or
		SCons.Script.ARGUMENTS.get('release','0') =='yes'):
		env.Append(CFLAGS=release_c_flags)
	return env
	
