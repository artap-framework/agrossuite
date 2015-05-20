PROJECT(${AGROS_LIBRARY} C CXX)
IF(MSVC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DAGROS_LIBRARY_DLL")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_LIBRARY_DLL")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
IF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
# paralution
INCLUDE_DIRECTORIES(${PARALUTION_INCLUDE_DIR})

# gmsh
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/gmsh/Common)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/gmsh/Geo)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/gmsh/Graphics)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/gmsh/Mesh)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/gmsh/Numeric)

# languages
# message("Translations")
FILE(GLOB lang_files "${CMAKE_HOME_DIRECTORY}/resources_source/lang/*.ts")
foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  add_custom_target(${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.ts ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm)

  # message("\tlrelease ${basefile}")

  ADD_CUSTOM_COMMAND(
        COMMAND   lrelease ARGS ${LANGFILE} -qm ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm
        OUTPUT   ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm)
endforeach()

# XSDCXX
# message("XML cpp/h files")
FILE(GLOB xsd_files "${CMAKE_HOME_DIRECTORY}/resources/xsd/*.xsd")
foreach(XSD_FILE ${xsd_files})
  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
  add_custom_target(${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h)

  # message("\txsdcxx ${basefile}")

  ADD_CUSTOM_COMMAND(
        COMMAND  ${XSD_BIN} ARGS cxx-tree --std c++11 --generate-ostream --hxx-suffix .h --cxx-suffix .cpp --root-element-first --generate-polymorphic --output-dir ${CMAKE_HOME_DIRECTORY}/resources_source/classes --generate-serialization ${XSD_FILE}
        DEPENDS ${CMAKE_HOME_DIRECTORY}/resources/xsd/${basefile}.xsd
        OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h)
endforeach()

# cython
# message("Cython files - Agros2D")
# message("\tcython agros2d.pyx")
add_custom_target(CYTHON_AGROS2D ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    COMMAND C:/Python34/Scripts/cython ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ELSE()
  ADD_CUSTOM_COMMAND(
    COMMAND cython3 ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ENDIF()

SET(SOURCES
    # mesh/meshgenerator_gmsh.cpp
    # sceneview_vtk2d.cpp
    #moduledialog.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/form_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/material_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/structure_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/test_xml.cpp
    chartdialog.cpp
    confdialog.cpp
    datatable.cpp
    examplesdialog.cpp
    gui/common.cpp
    gui/groupbox.cpp
    gui/htmledit.cpp
    gui/imageloader.cpp
    gui/physicalfield.cpp
    gui/scenewidget.cpp
    gui/valuedatatabledialog.cpp
    gui/valuelineedit.cpp
    gui/valuetimedialog.cpp
    infowidget.cpp
    logview.cpp
    mainwindow.cpp
    materialbrowserdialog.cpp
    mesh/agros_manifold.cpp
    mesh/meshgenerator.cpp
    mesh/meshgenerator_cubit.cpp
    mesh/meshgenerator_triangle.cpp
    optilab/optilab.cpp
    optilab/optilab_data.cpp
    optilab/optilab_multi.cpp
    optilab/optilab_single.cpp
    parser/lex.cpp
    particle/particle_tracing.cpp
    postprocessorview.cpp
    preprocessorview.cpp
    problemdialog.cpp
    pythonlab/pyfield.cpp
    pythonlab/pygeometry.cpp
    pythonlab/pyparticletracing.cpp
    pythonlab/pyproblem.cpp
    pythonlab/python_unittests.cpp
    pythonlab/pythonengine_agros.cpp
    pythonlab/pyview.cpp
    pythonlab/remotecontrol.cpp
    resultsview.cpp
    scene.cpp
    scenebasic.cpp
    scenebasicselectdialog.cpp
    sceneedge.cpp
    scenelabel.cpp
    scenemarker.cpp
    scenemarkerdialog.cpp
    scenemarkerselectdialog.cpp
    scenenode.cpp
    scenetransformdialog.cpp
    sceneview_common.cpp
    sceneview_common2d.cpp
    sceneview_common3d.cpp
    sceneview_geometry.cpp
    sceneview_geometry_chart.cpp
    sceneview_mesh.cpp
    sceneview_particle.cpp
    sceneview_post.cpp
    sceneview_post2d.cpp
    sceneview_post3d.cpp
    solver/bdf2.cpp
    solver/coupling.cpp
    solver/estimators.cpp
    solver/field.cpp
    solver/marker.cpp
    solver/module.cpp
    solver/plugin_interface.cpp
    solver/problem.cpp
    solver/problem_config.cpp
    solver/solutionstore.cpp
    solver/solutiontypes.cpp
    solver/solver.cpp
    solver/solver_nonlinear.cpp
    solver/solver_transient.cpp
    solver/linear_solver.cpp
    solver/linear_solver_external.cpp
    solver/weak_form.cpp
    util/conf.cpp
    util/dxf_filter.cpp
    util/enums.cpp
    util/form_interface.cpp
    util/form_script.cpp
    util/global.cpp
    util/loops.cpp
    util/memory_monitor.cpp
    util/xml.cpp
    value.cpp
    videodialog.cpp
  )
    
SOURCE_GROUP(
  "Source Files\\_Commented Out" FILES 
  # mesh/meshgenerator_gmsh.cpp
  # sceneview_vtk2d.cpp
  #moduledialog.cpp
)

SOURCE_GROUP(
  "Source Files\\Generated Classes" FILES 
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/form_xml.cpp
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/material_xml.cpp
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/structure_xml.cpp
  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/test_xml.cpp
)

SOURCE_GROUP(
  "Source Files\\GUI" FILES 
  gui/common.cpp
  gui/groupbox.cpp
  gui/htmledit.cpp
  gui/imageloader.cpp
  gui/physicalfield.cpp
  gui/scenewidget.cpp
  gui/valuedatatabledialog.cpp
  gui/valuelineedit.cpp
  gui/valuetimedialog.cpp
)

SOURCE_GROUP(
  "Source Files\\Mesh" FILES 
  mesh/agros_manifold.cpp
  mesh/meshgenerator.cpp
  mesh/meshgenerator_cubit.cpp
  mesh/meshgenerator_triangle.cpp
)

SOURCE_GROUP(
  "Source Files\\Optilab" FILES 
  optilab/optilab.cpp
  optilab/optilab_data.cpp
  optilab/optilab_multi.cpp
  optilab/optilab_single.cpp
)

SOURCE_GROUP(
  "Source Files\\PythonLab" FILES 
  pythonlab/pyfield.cpp
  pythonlab/pygeometry.cpp
  pythonlab/pyparticletracing.cpp
  pythonlab/pyproblem.cpp
  pythonlab/python_unittests.cpp
  pythonlab/pythonengine_agros.cpp
  pythonlab/pyview.cpp
  pythonlab/remotecontrol.cpp
)

SOURCE_GROUP(
  "Source Files\\Scene" FILES 
  scene.cpp
  scenebasic.cpp
  scenebasicselectdialog.cpp
  sceneedge.cpp
  scenelabel.cpp
  scenemarker.cpp
  scenemarkerdialog.cpp
  scenemarkerselectdialog.cpp
  scenenode.cpp
  scenetransformdialog.cpp
  sceneview_common.cpp
  sceneview_common2d.cpp
  sceneview_common3d.cpp
  sceneview_geometry.cpp
  sceneview_geometry_chart.cpp
  sceneview_mesh.cpp
  sceneview_particle.cpp
  sceneview_post.cpp
  sceneview_post2d.cpp
  sceneview_post3d.cpp
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
  solver/solutionstore.cpp
  solver/solutiontypes.cpp
  solver/solver.cpp
  solver/linear_solver.cpp
  solver/linear_solver_external.cpp
  solver/weak_form.cpp
)

SOURCE_GROUP(
  "Source Files\\Util" FILES 
  util/conf.cpp
  util/dxf_filter.cpp
  util/enums.cpp
  util/form_interface.cpp
  util/form_script.cpp
  util/global.cpp
  util/loops.cpp
  util/memory_monitor.cpp
  util/xml.cpp
)

SET(HEADERS
  # mesh/meshgenerator_gmsh.h
  # sceneview.h
  # sceneview_vtk2d.h
  #moduledialog.h
  chartdialog.h
  confdialog.h
  datatable.h
  examplesdialog.h
  gui/common.h
  gui/groupbox.h
  gui/htmledit.h
  gui/imageloader.h
  gui/physicalfield.h
  gui/scenewidget.h
  gui/valuedatatabledialog.h
  gui/valuelineedit.h
  gui/valuetimedialog.h
  infowidget.h
  logview.h
  mainwindow.h
  materialbrowserdialog.h
  mesh/agros_manifold.h
  mesh/meshgenerator.h
  mesh/meshgenerator_cubit.h
  mesh/meshgenerator_triangle.h
  optilab/optilab.h
  optilab/optilab_data.h
  optilab/optilab_multi.h
  optilab/optilab_single.h
  parser/lex.h
  particle/particle_tracing.h
  postprocessorview.h
  preprocessorview.h
  problemdialog.h
  pythonlab/pyfield.h
  pythonlab/pygeometry.h
  pythonlab/pyparticletracing.h
  pythonlab/pyproblem.h
  pythonlab/python_unittests.h
  pythonlab/pythonengine_agros.h
  pythonlab/pyview.h
  pythonlab/remotecontrol.h
  resultsview.h
  scene.h
  scenebasic.h
  scenebasicselectdialog.h
  sceneedge.h
  scenelabel.h
  scenemarker.h
  scenemarkerdialog.h
  scenemarkerselectdialog.h
  scenenode.h
  scenetransformdialog.h
  sceneview_common.h
  sceneview_common2d.h
  sceneview_common3d.h
  sceneview_data.h
  sceneview_geometry.h
  sceneview_geometry_chart.h
  sceneview_mesh.h
  sceneview_particle.h
  sceneview_post.h
  sceneview_post2d.h
  sceneview_post3d.h
  solver/bdf2.h
  solver/coupling.h
  solver/estimators.h
  solver/field.h
  solver/marker.h
  solver/module.h
  solver/plugin_interface.h
  solver/problem.h
  solver/problem_config.h
  solver/solutionstore.h
  solver/solutiontypes.h
  solver/solver.h
  solver/solver_nonlinear.h
  solver/solver_transient.h
  solver/linear_solver.h
  solver/linear_solver_external.h
  solver/weak_form.h
  util/conf.h
  util/constants.h
  util/dxf_filter.h
  util/enums.h
  util/form_interface.h
  util/form_script.h
  util/global.h
  util/loops.h
  util/memory_monitor.h
  util/xml.h
  value.h
  videodialog.h
)
  
SOURCE_GROUP(
  "Header Files\\_Commented Out" FILES 
  # mesh/meshgenerator_gmsh.h
  # sceneview.h
  # sceneview_vtk2d.h
  #moduledialog.h
)


SOURCE_GROUP(
  "Header Files\\GUI" FILES 
  gui/common.h
  gui/groupbox.h
  gui/htmledit.h
  gui/imageloader.h
  gui/physicalfield.h
  gui/scenewidget.h
  gui/valuedatatabledialog.h
  gui/valuelineedit.h
  gui/valuetimedialog.h
)

SOURCE_GROUP(
  "Header Files\\Mesh" FILES 
  mesh/agros_manifold.h
  mesh/meshgenerator.h
  mesh/meshgenerator_cubit.h
  mesh/meshgenerator_triangle.h
)

SOURCE_GROUP(
  "Header Files\\Optilab" FILES 
  optilab/optilab.h
  optilab/optilab_data.h
  optilab/optilab_multi.h
  optilab/optilab_single.h
)

SOURCE_GROUP(
  "Header Files\\PythonLab" FILES 
  pythonlab/pyfield.h
  pythonlab/pygeometry.h
  pythonlab/pyparticletracing.h
  pythonlab/pyproblem.h
  pythonlab/python_unittests.h
  pythonlab/pythonengine_agros.h
  pythonlab/pyview.h
  pythonlab/remotecontrol.h
)

SOURCE_GROUP(
  "Header Files\\Scene" FILES 
  scene.h
  scenebasic.h
  scenebasicselectdialog.h
  sceneedge.h
  scenelabel.h
  scenemarker.h
  scenemarkerdialog.h
  scenemarkerselectdialog.h
  scenenode.h
  scenetransformdialog.h
  sceneview_common.h
  sceneview_common2d.h
  sceneview_common3d.h
  sceneview_data.h
  sceneview_geometry.h
  sceneview_geometry_chart.h
  sceneview_mesh.h
  sceneview_particle.h
  sceneview_post.h
  sceneview_post2d.h
  sceneview_post3d.h
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
  solver/solutionstore.h
  solver/solutiontypes.h
  solver/solver.h
  solver/solver_external.h
  solver/weak_form.h
)

SOURCE_GROUP(
  "Header Files\\Util" FILES 
  util/conf.h
  util/constants.h
  util/dxf_filter.h
  util/enums.h
  util/form_interface.h
  util/form_script.h
  util/global.h
  util/loops.h
  util/memory_monitor.h
  util/xml.h
)

SET(RESOURCES ../resources_source/resources.qrc)
QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${RESOURCES_RCC})
ADD_DEPENDENCIES(${PROJECT_NAME} CYTHON_AGROS2D)


foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

foreach(XSD_FILE ${xsd_files})
  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets Svg UiTools OpenGL)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${PYTHONLAB_LIBRARY} ${AGROS_UTIL} ${TRIANGLE_LIBRARY} ${CTEMPLATE_LIBRARY} ${DXFLIB_LIBRARY} ${POLY2TRI_LIBRARY} ${QCUSTOMPLOT_LIBRARY} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES} ${ZLIB_LIBRARY} ${XSD_LIBRARY} ${XERCES_LIBRARY} ${DEAL_II_LIBRARIES} ${PYTHON_LIBRARIES} ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES} ${UMFPACK_LIBRARY} ${AMD_LIBRARY} ${SuiteSparse_config_LIBRARY} ${PARALUTION_LIBRARY})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
