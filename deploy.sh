#!/usr/bin/python3

from shutil import copytree, ignore_patterns
import os
import shutil

# dest = 'Agros.AppDir'
dest = 'appimage'

# rm files
shutil.rmtree(dest) 

# create dirs
os.mkdir(dest)
os.mkdir(dest + '/bin')
# os.mkdir(dest + '/lib')
os.mkdir(dest + '/share')
os.mkdir(dest + '/share/agrossuite')

# copy binary files
shutil.copy('agros', dest + '/bin/agros')
shutil.copytree('resources', dest + '/share/agrossuite/resources')
shutil.copytree('libs', dest + '/lib', ignore=ignore_patterns('*.a'))
# os.symlink(os.readlink('dealii/build/lib/libdeal_II.so'), dest + '/libs/libdeal_II.so')    
# shutil.copy('dealii/build/lib/libdeal_II.so.9.4.2', dest + '/libs/libdeal_II.so.9.4.2')

# copy resources
shutil.copytree('resources_source/appimage', dest + '/', dirs_exist_ok=True)

# strip
os.system("strip " + dest + "/bin/*")
os.system("strip " + dest + "/lib/*")

# qt path
os.system("PATH=/usr/bin:$PATH")
os.system("cd " + dest)
os.system("linuxdeployqt-continuous-x86_64.AppImage --unsupported-allow-new-glibc agros")
os.system("cd ..")
