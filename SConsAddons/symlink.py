##
# builder for symlink creation

from SCons import Builder

def add_symlink(env):
	lib_builder = Builder.Builder(action = "cd ${SOURCE.dir} && ln -s ${SOURCE.file} ${TARGET.file}", chdir = False)
	env.Append(BUILDERS = {"LibrarySymlink" : lib_builder})
