#!/bin/sh

#matlab -nodesktop -nosplash -r "cd ~/Projects/agros2d-dealii/agros-swig/matlab; v = agros.version(); disp(v); exit;"
/opt/matlab-R2015a/bin/glnxa64/MATLAB -nodesktop -nosplash -r "cd ~/Projects/agros2d-dealii/agros-swig/matlab; agros.initSingleton(); p = agros.Problem(); exit;"
