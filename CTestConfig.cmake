## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "Agros2D")
set(CTEST_NIGHTLY_START_TIME "01:00:00 UTC")

set(CTEST_DROP_METHOD "https")
set(CTEST_DROP_SITE "edison.fel.zcu.cz:999")
set(CTEST_DROP_LOCATION "/submit.php?project=Agros2D")
set(CTEST_DROP_SITE_CDASH TRUE)
