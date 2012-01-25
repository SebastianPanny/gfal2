##
# Util tools for versions and project parameters management
#


def get_project_vars_from_env(env):
	if(isinstance(env["PROJECT_VARS"], dict) is False):
		raise Exception(" Need a call to Setup_project_environment before ")
	return env["PROJECT_VARS"]
	
	
	
def Setup_project_environment(env, Name="example", Version="1.0.0", Release="1.0", License="GPL"):
	
	v = str(Version).split(".")
	i = 0
	r = []
	while(i <3 ):
		tmp = ((i < len(v)) and v[i]) or "0"
		r.append(tmp)
		i+=1
	
	env["PROJECT_VARS"] = { "NAME" : Name, "VERSION_MAJOR" : r[0], "VERSION_MINOR" : r[1], "VERSION_PATCH" : r[2], "RELEASE" : Release, "LICENSE" : License}
