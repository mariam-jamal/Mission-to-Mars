#!/usr/bin/python

import sys
import os
import os.path
import shutil


def printhelp():
    print """Syntax: python """ + sys.argv[0] + """ myProjectName targetPath

Creates a Dorobo32 Eclipse project named myProjectName in a subdirectory 
named myProjectName in targetPath. 
After running the script, start Eclipse and select 
*File/Import/General/Existing Pojects into Workspace* to import the project
in the directory targetPath/myProjectName into the Eclipse workspace.

"""


def change_line(filename, old_string, new_string):
    with open(filename) as f_in:
        contents = f_in.read()
    if not old_string in contents:
    	print "Warning: '{old_string}' not found in '{filename}'.".format(**locals())
    contents = contents.replace(old_string, new_string)
    with open(filename, 'w') as f_out:
        print "Patching '{filename}': '{old_string}' -> '{new_string}'".format(**locals())
        f_out.write(contents)


if __name__ == "__main__":
    print "Create Dorobo32 project from template"
    if len(sys.argv) != 3:
    	printhelp()
        print "Error: Invalid number of command line arguments"
        sys.exit(1)

    project_name = sys.argv[1]
    target_path = sys.argv[2]
    project_path = os.path.join(target_path, project_name)
    template_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'template')

    print "Project name:  " + project_name
    print "Project path:  " + project_path
    print "Template path: " + template_path
    if os.path.isdir(project_path):
    	print "Error: project path already exists - remove that directory!"
    	sys.exit(1)
#    os.makedirs(project_path)
    shutil.copytree(template_path, project_path)
    change_line(os.path.join(project_path, '.project'), 'dorobo32-template', project_name)
