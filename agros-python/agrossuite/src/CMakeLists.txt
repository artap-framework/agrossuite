find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
qt_standard_project_setup()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../3rdparty/nsga2)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/build/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/bundled/boost-1.70.0/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/bundled/tbb-2018_U2/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/bundled/kokkos-3.7.00/core/src/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../dealii/bundled/kokkos-3.7.00/tpls/desul/include/)

INCLUDE_DIRECTORIES(${Qt6Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt6Network_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt6Core5Compat_INCLUDE_DIRS})

if(WIN32)

set(AGROS_PYTHON ${CMAKE_HOME_DIRECTORY}/../../libs/agros_python.lib)
set(AGROS_LIBRARY ${CMAKE_HOME_DIRECTORY}/../../libs/agros_library.lib)
set(DEAL_LIBRARY ${CMAKE_HOME_DIRECTORY}/../../libs/deal_II.lib)

SET(ZLIB_LIBRARY "C:/agros_dependencies/source/zlib131/zlib-1.3.1/Release/zlib.lib")
SET(ZLIB_INCLUDE_DIR "C:/agros_dependencies/source/zlib131/zlib-1.3.1")

else()

set(AGROS_PYTHON ${CMAKE_HOME_DIRECTORY}/../../libs/libagros_python.so)

endif()

add_cython_target(_agros CXX PY3 OUTPUT_VAR X)
add_library(_agros MODULE ${x})
TARGET_LINK_LIBRARIES(_agros ${AGROS_PYTHON} ${AGROS_LIBRARY} ${DEAL_LIBRARY} ${ZLIB_LIBRARY}  Qt6::Core Qt6::Network)
python_extension_module(_agros)

install(TARGETS _agros LIBRARY DESTINATION src) 
install(FILES __init__.py DESTINATION src)
install(FILES agros.py DESTINATION src)
install(FILES vtk_tools.py DESTINATION src)
install(DIRECTORY ../../tests DESTINATION src)
install(DIRECTORY ${CMAKE_HOME_DIRECTORY}/../../libs DESTINATION src)