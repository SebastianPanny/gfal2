# -*- conding: utf-8 -*-


def get_rpm_changelog_from_file(filename = "CHANGELOG"):
	f= open(filename, 'r')
	d = f.readlines()
	f.close()
	return ''.join(d)
