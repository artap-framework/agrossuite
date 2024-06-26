PROJECT(${AGROS_GUI})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-gui)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)

SET(SOURCES main.cpp
    gui/about.cpp
    gui/chart.cpp
    gui/common.cpp
    gui/fielddialog.cpp
    gui/functiondialog.cpp
    gui/htmledit.cpp
    gui/infowidget.cpp
    gui/lineeditdouble.cpp
    gui/logwidget.cpp
    gui/other.cpp
    gui/parameterdialog.cpp
    gui/physicalfield.cpp
    gui/problemdialog.cpp
    gui/recipedialog.cpp
    gui/resultsview.cpp

    gui/systemoutput.cpp
    gui/valuedatatabledialog.cpp
    gui/valuelineedit.cpp
    gui/valuetimedialog.cpp
    gui/checkversion.cpp
    app/chartdialog.cpp
    app/confdialog.cpp
    app/examplesdialog.cpp
    app/mainwindow.cpp
    app/materialbrowserdialog.cpp
    app/postprocessorview_chart.cpp
    app/postprocessorview.cpp
    app/postprocessorview_mesh.cpp
    #app/postprocessorview_particletracing.cpp
    app/postprocessorview_post2d.cpp
    app/postprocessorview_post3d.cpp
    app/preprocessorview.cpp
    app/scenebasicselectdialog.cpp
    app/scenemarkerdialog.cpp
    app/scenemarkerselectdialog.cpp
    app/scenegeometrydialog.cpp
    app/scenetransformdialog.cpp
    app/sceneview_common2d.cpp
    app/sceneview_common3d.cpp
    app/sceneview_common.cpp
    app/sceneview_geometry.cpp
    app/sceneview_geometry_simple.cpp
    app/sceneview_mesh.cpp
    #app/sceneview_particle.cpp
    app/sceneview_post2d.cpp
    app/sceneview_post3d.cpp
    app/sceneview_post.cpp
    app/scriptgenerator_utils.cpp
    app/scriptgeneratordialog.cpp
    # app/sceneview_vtk2d.cpp
    app/videodialog.cpp    
    optilab/optilab.cpp
    optilab/study_dialog.cpp
    optilab/study_methoddialog.cpp)

SET(HEADERS gui/about.h
    gui/chart.h
    gui/common.h
    gui/fielddialog.h
    gui/functiondialog.h
    gui/htmledit.h
    gui/infowidget.h
    gui/lineeditdouble.h
    gui/logwidget.h
    gui/other.h
    gui/parameterdialog.h
    gui/physicalfield.h
    gui/problemdialog.h
    gui/recipedialog.h
    gui/resultsview.h

    gui/systemoutput.h
    gui/valuedatatabledialog.h
    gui/valuelineedit.h
    gui/valuetimedialog.h
    gui/checkversion.h
    app/chartdialog.h
    app/confdialog.h
    app/examplesdialog.h
    app/mainwindow.h
    app/materialbrowserdialog.h
    app/postprocessorview_chart.h
    app/postprocessorview.h
    app/postprocessorview_mesh.h
    #app/postprocessorview_particletracing.h
    app/postprocessorview_post2d.h
    app/postprocessorview_post3d.h
    app/preprocessorview.h
    app/scenebasicselectdialog.h
    app/scenemarkerdialog.h
    app/scenemarkerselectdialog.h
    app/scenegeometrydialog.h
    app/scenetransformdialog.h
    app/sceneview_common2d.h
    app/sceneview_common3d.h
    app/sceneview_common.h
    app/sceneview_data.h
    app/sceneview_geometry.h
    app/sceneview_geometry_simple.h
    app/sceneview_mesh.h
    #app/sceneview_particle.h
    app/sceneview_post2d.h
    app/sceneview_post3d.h
    app/sceneview_post.h
    app/scriptgenerator_utils.h
    app/scriptgeneratordialog.h
    # app/sceneview_vtk2d.h
    app/videodialog.h    
    optilab/optilab.h
    optilab/study_dialog.h
    optilab/study_methoddialog.h)

# images
SET(RESOURCES_ICON ../resources_source/resources.rc)
SET(RESOURCES ../resources_source/resources.qrc
              # ../resources_source/themes/dark/darkstyle.qrc
)
QT_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})


find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Svg REQUIRED)
find_package(Qt6 COMPONENTS SvgWidgets REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)
find_package(Qt6 COMPONENTS LinguistTools REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED) 
find_package(Qt6 COMPONENTS OpenGL REQUIRED)
find_package(Qt6 COMPONENTS Charts REQUIRED)

IF(WIN32)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /bigobj")
    find_package(OpenGL REQUIRED)
ENDIF(WIN32)


# translations
set(TS_FILES
    "cs_CZ.ts"
    "en_US.ts"
    "pl_PL.ts"
    "plugin_cs_CZ.ts"
    "plugin_en_US.ts"
    "plugin_pl_PL.ts")
list(TRANSFORM TS_FILES PREPEND ${CMAKE_HOME_DIRECTORY}/resources_source/lang/)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_HOME_DIRECTORY}/resources_source/lang/")


IF(WIN32)
set(WINDOWS_FLAG WIN32)
ENDIF(WIN32)

ADD_EXECUTABLE(${PROJECT_NAME} ${WINDOWS_FLAG} ${SOURCES} ${HEADERS} ${GUI} main.cpp  ${RESOURCES_RCC} ${RESOURCES_ICON} ${QM_FILES})


# translations
qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES})
qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PLUGINS} ${QT_QTMAIN_LIBRARY} ${AGROS_LIBRARY} ${QCUSTOMPLOT_LIBRARY} Qt6::OpenGLWidgets Qt6::Svg Qt6::SvgWidgets OpenGL::GL Qt6::OpenGL Qt6::Charts)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME agros)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
