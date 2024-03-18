#/bin/sh

cd agros-python 
python3 -m venv env
source env/bin/activate
pip install cython
pip install scikit-build
pip install pytest
pip install numpy
# compile
pip install agrossuite/.
cd tests
# pytest --junitxml=../../report.xml --disable-warnings --durations=0 -v
pytest --disable-warnings --durations=0 -v
