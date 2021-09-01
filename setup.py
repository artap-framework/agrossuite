import setuptools
import distutils
import os
import datetime
from glob import glob
import shutil

base_dir = os.path.abspath(os.path.dirname(__file__))

try:
    shutil.rmtree('dist')
except:
    pass
try:
    shutil.rmtree('build')
except:
    pass
try:
    shutil.rmtree('agrossuite/resources')
except:
    pass
try:
    shutil.rmtree('agrossuite/libs')
except:
    pass
try:
    shutil.rmtree('agrossuite/__pycache__')
except:
    pass
    
# copy files
# distutils.file_util.copy_file("libs/_agros.so", "agrossuite/_agros.so")
# distutils.dir_util.copy_tree("resources", "agrossuite/resources")

# libs
if not os.path.exists("agrossuite/libs"):
    os.makedirs("agrossuite/libs")
for (dirpath, dirnames, filenames) in os.walk("libs"):
    for file in filenames:
        if file == "_agros.so":
            continue
        ext = os.path.splitext(file)[-1].lower()
        if ext == ".so":
            distutils.file_util.copy_file("libs/" + file, "agrossuite/libs/" + file)

# deal
distutils.file_util.copy_file("dealii/build/lib/libdeal_II.so.9.2.0", "agrossuite/libs/libdeal_II.so.9.2.0")
# os.symlink("libdeal_II.so.9.0.1", "agrossuite/libs/libdeal_II.so")

# strip libraries
os.system("strip " + base_dir + "/agrossuite/libs/*")
#os.remove(base_dir + "/agrossuite/libs/libagros_plugin_dek.so")
os.system("strip " + base_dir + "/agrossuite/_agros.so")
   
# long description
with open(base_dir + "/README", "r") as fh:
    long_description = fh.read()

dt = datetime.datetime.now()

setuptools.setup(
    name="agrossuite",
    version="{}.{}".format(dt.strftime('%Y.%m.%d'), dt.hour*60*60+dt.minute*60+dt.second),
    author=u"Agros Suite",
    author_email="karban@kte.zcu.cz",
    description="Multiplatform application for the solution of physical problems based on the deal.II library",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://www.agros2d.org/",
    python_requires='>3.8',
    license="License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
    packages=setuptools.find_packages(),
    include_package_data=True,
    # data_files=local_data_files,
    # install_requires=requirements,    
    classifiers=[
        "Intended Audience :: Science/Research",
        "Operating System :: POSIX :: Linux",
        "Topic :: Scientific/Engineering",
        'Programming Language :: Python :: 3.8',
        "License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
        "Operating System :: OS Independent",
    ],
)
