import setuptools
import distutils
import os
from glob import glob
import shutil

base_dir = os.path.abspath(os.path.dirname(__file__))

# copy files
distutils.file_util.copy_file("agros.so", "agrossuite/agros.so")
distutils.dir_util.copy_tree("resources", "agrossuite/resources")
if not os.path.exists("agrossuite/libs"):
    os.makedirs("agrossuite/libs")
for (dirpath, dirnames, filenames) in os.walk("libs"):
    for file in filenames:
        ext = os.path.splitext(file)[-1].lower()
        if ext == ".so":
            distutils.file_util.copy_file("libs/" + file, "agrossuite/libs/" + file)

# strip libraries
os.system("strip {}".format(base_dir + "agrossuite/libs/*.so"))
os.system("strip agrossuite/agros.so")
    
# long description
with open(base_dir + "/README", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="agrossuite",
    version="2020.2.8.5",
    author=u"Agros Suite",
    author_email="karban@kte.zcu.cz",
    description="Multiplatform application for the solution of physical problems based on the deal.II library",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://www.agros2d.org/",
    python_requires='>3.7',
    license="License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
    packages=setuptools.find_packages(),
    include_package_data=True,
    # data_files=local_data_files,
    # install_requires=requirements,    
    classifiers=[
        "Intended Audience :: Science/Research",
        "Operating System :: POSIX :: Linux",
        "Topic :: Scientific/Engineering",
        'Programming Language :: Python :: 3.7',
        "License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
        "Operating System :: OS Independent",
    ],
)
