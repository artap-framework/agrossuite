#/bin/sh

cd agros-python 
python3 -m venv env
source env/bin/activate
pip install cython
pip install scikit-build
cd agrossuite
pip install .
cd _skbuild/linux-x86_64-3.12/setuptools/lib.linux-x86_64-cpython-312/agrossuite/tests
# pytest-3 --junitxml=../../report.xml --disable-warnings --durations=0 -v
pytest-3 --disable-warnings --durations=0 -v
