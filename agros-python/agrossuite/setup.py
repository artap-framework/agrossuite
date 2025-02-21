import setuptools
import os
import datetime
from glob import glob
import shutil
from skbuild import setup

# long description
# with open(base_dir + "/README", "r") as fh:
with open("README", "r") as fh:
    long_description = fh.read()

dt = datetime.datetime.now()

setup(
    name="agrossuite",
    # version="{}.{}".format(dt.strftime('%Y.%m.%d'), dt.hour*60*60+dt.minute*60+dt.second),
    version = "{}".format(dt.strftime('%Y.%m.%d.%H%M')),
    author=u"Agros Suite",
    author_email="karban@fel.zcu.cz",
    description="Multiplatform application for the solution of physical problems based on the deal.II library",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://www.agros2d.org/",
    python_requires='>3.10',
    license="License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
    packages=["agrossuite"],
    package_dir={"agrossuite": "src"},
    include_package_data=True,
    # data_files=[("", ["tests/*"])],
    # data_files=local_data_files,
    # install_requires=requirements,    
    classifiers=[
        "Intended Audience :: Science/Research",
        "Operating System :: POSIX :: Linux",
        "Topic :: Scientific/Engineering",
        'Programming Language :: Python :: 3.11',
        "License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)",
        "Operating System :: OS Independent",
    ],
)
