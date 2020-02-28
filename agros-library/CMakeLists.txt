PROJECT(${AGROS_LIBRARY} C CXX)
IF(MSVC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DAGROS_LIBRARY_DLL")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_LIBRARY_DLL")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga3)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/limbo)

# languages
message("Translations")
FILE(GLOB lang_files "${CMAKE_HOME_DIRECTORY}/resources_source/lang/*.ts")
foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  add_custom_target(${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.ts ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.qm)

  # message("\tlrelease ${basefile}")

  ADD_CUSTOM_COMMAND(
        COMMAND   lrelease ARGS ${LANGFILE} -qm ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.qm
        OUTPUT   ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.qm)
endforeach()

SET(SOURCES
    # sceneview_vtk2d.cpp
    # ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
    # ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
    datatable.cpp
    logview.cpp
    mesh/meshgenerator.cpp
    mesh/meshgenerator_triangle.cpp
    mesh/meshgenerator_gmsh.cpp
    parser/lex.cpp
    optilab/doe.cpp
    optilab/study.cpp
    optilab/study_sweep.cpp
    optilab/study_nsga2.cpp
    optilab/study_nsga3.cpp
    optilab/study_bayesopt.cpp
    optilab/study_nlopt.cpp
    optilab/study_limbo.cpp
    optilab/parameter.cpp
    optilab/functional.cpp
    particle/particle_tracing.cpp
    particle/particle_forces.cpp
    scene.cpp
    scenebasic.cpp
    sceneedge.cpp
    scenelabel.cpp
    scenemarker.cpp
    scenenode.cpp
    solver/bdf2.cpp
    solver/coupling.cpp
    solver/estimators.cpp
    solver/field.cpp
    solver/marker.cpp
    solver/module.cpp
    solver/plugin_interface.cpp
    solver/problem.cpp
    solver/problem_config.cpp
    solver/problem_result.cpp
    solver/problem_function.cpp
    solver/problem_parameter.cpp
    solver/solutionstore.cpp
    solver/solutiontypes.cpp
    solver/solver.cpp
    solver/solver_utils.cpp
    solver/solver_nonlinear.cpp
    solver/linear_solver.cpp
    solver/linear_solver_external.cpp
    solver/weak_form.cpp
    util/conf.cpp
    util/dxf_filter.cpp
    util/enums.cpp
    util/global.cpp
    util/loops.cpp
    util/util.cpp
    util/util_expr.cpp
    util/point.cpp
    util/system_utils.cpp
    util/spline.cpp
    util/table.cpp
    value.cpp
  )   

SOURCE_GROUP(
  "Source Files\\Generated Classes" FILES 
  # ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
)

SOURCE_GROUP(
  "Source Files\\Mesh" FILES 
  mesh/meshgenerator.cpp
  mesh/meshgenerator_triangle.cpp
  mesh/meshgenerator_gmsh.cpp
)

SOURCE_GROUP(
  "Source Files\\Scene" FILES 
  scene.cpp
  scenebasic.cpp
  sceneedge.cpp
  scenelabel.cpp
  scenemarker.cpp  
  scenenode.cpp
)

SOURCE_GROUP(
  "Source Files\\Solver" FILES 
  solver/bdf2.cpp
  solver/coupling.cpp
  solver/estimators.cpp
  solver/field.cpp
  solver/marker.cpp
  solver/module.cpp
  solver/plugin_interface.cpp
  solver/problem.cpp
  solver/problem_config.cpp
  solver/problem_result.cpp
  solver/problem_function.cpp
  solver/problem_parameter.cpp
  solver/solutionstore.cpp
  solver/solutiontypes.cpp
  solver/solver.cpp
  solver/solver_utils.cpp
  solver/solver_nonlinear.cpp
  solver/linear_solver.cpp
  solver/linear_solver_external.cpp
  solver/weak_form.cpp
)

SOURCE_GROUP(
  "Source Files\\Util" FILES 
  util/conf.cpp
  util/dxf_filter.cpp
  util/enums.cpp
  util/global.cpp
  util/loops.cpp
  util/xml.cpp
)

SET(HEADERS
  # sceneview.h
  # sceneview_vtk2d.h  
  datatable.h
  logview.h
  mesh/meshgenerator.h
  mesh/meshgenerator_triangle.h
  mesh/meshgenerator_gmsh.h
  parser/lex.h
  optilab/doe.h
  optilab/study.h
  optilab/study_sweep.h
  optilab/study_nsga2.h
  optilab/study_nsga3.h
  optilab/study_bayesopt.h
  optilab/study_nlopt.h
  optilab/study_limbo.h
  optilab/parameter.h
  optilab/functional.h
  particle/particle_tracing.h
  particle/particle_forces.h
  scene.h
  scenebasic.h
  sceneedge.h
  scenelabel.h
  scenemarker.h
  scenenode.h
  solver/bdf2.h
  solver/coupling.h
  solver/estimators.h
  solver/field.h
  solver/marker.h
  solver/module.h
  solver/plugin_interface.h
  solver/problem.h
  solver/problem_config.h
  solver/problem_result.h
  solver/problem_function.h
  solver/problem_parameter.h
  solver/solutionstore.h
  solver/solutiontypes.h
  solver/solver.h
  solver/solver_utils.h
  solver/solver_nonlinear.h
  solver/linear_solver.h
  solver/linear_solver_external.h
  solver/weak_form.h
  util/conf.h
  util/constants.h
  util/dxf_filter.h
  util/enums.h
  util/global.h
  util/loops.h
  util/util.h
  util/util_expr.h
  util/point.h
  util/system_utils.h
  util/spline.h
  util/table.h
  value.h
)
  
SOURCE_GROUP(
  "Header Files\\_Commented Out" FILES 
  # sceneview.h
  # sceneview_vtk2d.h
)

SOURCE_GROUP(
  "Header Files\\Mesh" FILES 
  mesh/meshgenerator.h
  mesh/meshgenerator_triangle.h
)

SOURCE_GROUP(
  "Header Files\\Scene" FILES 
  scene.h
  scenebasic.h
  sceneedge.h
  scenelabel.h
  scenemarker.h
  scenenode.h
)

SOURCE_GROUP(
  "Header Files\\Solver" FILES 
  solver/bdf2.h
  solver/coupling.h
  solver/estimators.h
  solver/field.h
  solver/marker.h
  solver/module.h
  solver/plugin_interface.h
  solver/problem.h
  solver/problem_config.h
  solver/problem_result.h
  solver/problem_function.h
  solver/problem_parameter.h
  solver/solutionstore.h  
  solver/solutiontypes.h
  solver/solver.h
  solver/solver_utils.h
  solver/solver_nonlinear.h  
  solver/solver_external.h
  solver/weak_form.h
)

SOURCE_GROUP(
  "Header Files\\Util" FILES 
  util/conf.h
  util/constants.h
  util/dxf_filter.h
  util/enums.h
  util/global.h
  util/loops.h
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

#foreach(XSD_FILE ${xsd_files})
#  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
#  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
#endforeach()

# ${XSD_LIBRARY} ${XercesC_LIBRARIES} 
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${TRIANGLE_LIBRARY} ${DXFLIB_LIBRARY} ${POLY2TRI_LIBRARY} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY} ${ZLIB_LIBRARIES} ${ZLIB_LIBRARY} ${CURL_LIBRARIES} ${DEAL_II_LIBRARIES} ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES} ${AMD_LIBRARY} ${MATIO_LIBRARY} ${BAYESOPTLIB_LIBRARY} ${LIMBO_LIBRARY} ${NLOPTLIB_LIBRARY} ${NSGA2_LIBRARY} ${NSGA3_LIBRARY} ${CTEMPLATE_LIBRARY} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
