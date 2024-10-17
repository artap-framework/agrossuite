#/bin/sh

cd agros-python 
python3 -m venv env
source env/bin/activate
pip install setuptools --upgrade
pip install cython --upgrade
pip install scikit-build --upgrade
pip install pytest --upgrade
pip install pytest-rerunfailures --upgrade
pip install numpy --upgrade
# compile
pip install agrossuite/. -v
cd tests
# pytest --junitxml=../../report.xml --disable-warnings --durations=0 -v
pytest --disable-warnings --durations=0 -v --reruns 5 --only-rerun ValueError 
