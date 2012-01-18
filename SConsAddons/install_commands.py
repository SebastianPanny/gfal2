###
## Utility module for advanced install management
# 


import version_commands
import install_define_path
import symlink
import os

import SCons


def load_install_commands(env, args):
	symlink.add_symlink(env)
	install_define_path.configure_install_directories(env, args)
	Add_SharedLibraryInstall(env)




def Add_SharedLibraryInstall(env):
	def shared_library_install(env, source):
		symlink.add_symlink(env)
		res = []
		for i in source:
			version_major = version_commands.get_project_vars_from_env(env)["VERSION_MAJOR"] 
			version_minor = version_commands.get_project_vars_from_env(env)["VERSION_MINOR"] 
			version_patch = version_commands.get_project_vars_from_env(env)["VERSION_PATCH"] 
			major_file = "".join([i.name, ".", version_major])
			minor_file = "".join([major_file, ".", version_minor])
			full_file= "".join([minor_file, ".",version_patch])
			initial_file = os.path.join(install_define_path.get_install_dirs(env).get("LIB_DIR"),  full_file) 
			tmp = env.InstallAs(initial_file , source) 
			res += tmp 
			res += env.LibrarySymlink(major_file , tmp)  
			res += env.LibrarySymlink(minor_file , tmp)  
			res += env.LibrarySymlink(initial_file , tmp)  
		return res
	env.AddMethod(shared_library_install, "SharedLibraryInstall")
