find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
qt_standard_project_setup()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../3rdparty/nsga2)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/build/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/bundled/boost-1.70.0/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/bundled/tbb-2018_U2/include/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/bundled/kokkos-3.7.00/core/src/)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../../../dealii/bundled/kokkos-3.7.00/tpls/desul/include/)

INCLUDE_DIRECTORIES(${Qt6Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt6Network_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt6Core5Compat_INCLUDE_DIRS})

if (WIN32)
    set(AGROS_PYTHON ${CMAKE_HOME_DIRECTORY}/../../usr/lib/agros_python.lib)
    set(AGROS_LIBRARY ${CMAKE_HOME_DIRECTORY}/../../usr/lib/agros_library.lib)
    set(DEAL_LIBRARY ${CMAKE_HOME_DIRECTORY}/../../usr/lib/deal_II.lib)
    SET(ZLIB_LIBRARY "C:/agros_dependencies/Release/libs/zlib.lib")
    SET(ZLIB_INCLUDE_DIR "C:/agros_dependencies/include/zlib")
else()
    set(AGROS_PYTHON ${CMAKE_HOME_DIRECTORY}/../../usr/lib/libagros_python.so)
endif()

if (WIN32)
	add_custom_command(OUTPUT "${CMAKE_HOME_DIRECTORY}/src/_agros.cpp" COMMAND cython --cplus "${CMAKE_HOME_DIRECTORY}/src/_agros.pyx" -o "${CMAKE_HOME_DIRECTORY}/src/_agros.cpp" DEPENDS _agros.pyx)
	add_library(_agros MODULE _agros.cpp)
else()
	add_cython_target(_agros CXX PY3 OUTPUT_VAR X)
	add_library(_agros MODULE ${X})
endif()

if (WIN32)
    TARGET_LINK_LIBRARIES(_agros ${AGROS_PYTHON} ${AGROS_LIBRARY} ${DEAL_LIBRARY} ${ZLIB_LIBRARY}  Qt6::Core Qt6::Network)
else()
    TARGET_LINK_LIBRARIES(_agros ${AGROS_PYTHON} Qt6::Core Qt6::Network)
endif()

python_extension_module(_agros)

install(TARGETS _agros LIBRARY DESTINATION src) 
install(FILES __init__.py DESTINATION src)
install(FILES agros.py DESTINATION src)
install(FILES tools.py DESTINATION src)
install(DIRECTORY ${CMAKE_HOME_DIRECTORY}/../tests DESTINATION src PATTERN "__pycache__" EXCLUDE PATTERN ".pytest_cache" EXCLUDE)

if(WIN32)	
	file(GLOB DLL_FILES "${CMAKE_HOME_DIRECTORY}/../../usr/lib/*.dll")
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../../../install/Others/libifcoremd.dll DESTINATION src/lib)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../../../install/Others/libmmd.dll DESTINATION src/lib)
	install(FILES ${DLL_FILES} DESTINATION src/lib)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/agros_3rdparty_triangle.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/zlib.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/xerces-c_3_1.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/agros_library.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/agros_python.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/Qt6Core.dll DESTINATION src)
	install(FILES ${CMAKE_HOME_DIRECTORY}/../../usr/bin/Qt6Core5Compat.dll DESTINATION src)
else()
 	install(DIRECTORY ${CMAKE_HOME_DIRECTORY}/../../usr/lib DESTINATION src)
endif()

