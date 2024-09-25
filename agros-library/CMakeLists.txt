PROJECT(${AGROS_LIBRARY} C CXX)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -DAGROS_LIBRARY_DLL")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)

SET(SOURCES
    # ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
    # ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
    datatable.cpp
    logview.cpp
    mesh/meshgenerator.cpp
    mesh/meshgenerator_triangle.cpp
    parser/lex.cpp
    optilab/study.cpp
    optilab/study_sweep.cpp
    optilab/study_nsga2.cpp
    optilab/study_bayesopt.cpp
    optilab/study_nlopt.cpp
    optilab/study_openga.cpp
    optilab/study_model.cpp
    optilab/parameter.cpp
    optilab/goal_function.cpp
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
    util/util.cpp
    util/util_expr.cpp
    util/point.cpp
    util/system_utils.cpp
    util/spline.cpp
    util/table.cpp
    util/script_generator.cpp
    util/loops.cpp
    util/loops.h
    value.cpp
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WINDOWS_EXPORT_ALL_SYMBOLS)
endif()

foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

DEAL_II_SETUP_TARGET(${PROJECT_NAME} RELEASE)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_LIBRARIES} ${TRIANGLE_LIBRARY} ${DXFLIB_LIBRARY} ${QUAZIP_LIBRARY} ${ZLIB_LIBRARIES} ${ZLIB_LIBRARY} ${CURL_LIBRARIES} ${BLAS_LIBRARIES} ${AMD_LIBRARY} ${MATIO_LIBRARY} ${BAYESOPTLIB_LIBRARY} ${NLOPTLIB_LIBRARY} ${NSGA2_LIBRARY} ${CTEMPLATE_LIBRARY} Qt6::Core Qt6::Network Qt6::Core5Compat)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
