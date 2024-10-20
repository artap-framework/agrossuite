
1) Preparing for compilation

1.1) Download and install cmake for Windows: https://cmake.org/download/
1.2) Download and install MS Visual Studio 2015 (Community Edition is enough). Don't forget, after MSVC installation, to continue download and install necessary C++ packages (compiler and tools) - in File -> New
1.3 Download and instal Doxygen for Windows https://sourceforge.net/projects/doxygen/files/snapshots/
Put it into the folder C:\Doxygen\
Add path to binaries (dogygen.exe) to the Windows system variable PATH

2) Dependencies

2.1) Compile deal.ii in Agros Suite subfolder dealii (we expect, that Agros suite project is, for example in C:\agros2d folder):
In the Agros2D folder (for example C:\agros2d) prepare a copy of CMake.vars.Windows to file CMake.vars. There You can make Your individual settings.
Into this CMake.vars add a variable settings (at the beginning of the file, after dependencies folder settings):

      SET(BOOST_ROOT "your_agros_folder\\dealii\\bundled\\boost-1.62.0")

A continue with commands:
 
      cd C:\agros2d\dealii
      mkdir build
      cd build
      cmake -G "Visual Studio 14 Win64" -D CMAKE_INSTALL_PREFIX=your_agros_folder\dealii\install .. -DDEAL_II_FORCE_BUNDLED_BOOST:BOOL=TRUE

2.2) Improve path to the Windows dependecies, for example "C:\agros_dependecies" at the beginning of the file "CMake.vars.Windows". In this case, the 32-bit dependencies in C:\agros_dependecies\32 and the 64-bit ones in C:\agros_dependecies\64

The beginning of the file "CMake.vars.Windows":
# SET dependencies directory.
# If individual dependencies are not all in this common directory, it has to be changed further down.
SET(DEPENDENCIES_DIR "C:/agros_dependecies")
SET(DEPENDENCIES_DIR_WITH_SLASHES "C:\\agros_dependecies")

3) Needed to download | build (in x86 || x64 depending on the target platform of Agros2D):

3.0) Qt

  - Qt for the correct platform (x86 / x64), for VS 2015, with OpenGL
  https://www.qt.io/download-open-source/

---- NOT INSTALL - bundled now ----
3.1) Xerces & XSD

  - http://www.codesynthesis.com/products/xsd/download.xhtml
  - XSD version 4.0+ necessary (for C++11 support)
-----------------------------------

3.4) ZLIB

  - x86: http://gnuwin32.sourceforge.net/packages/zlib.htm
  - x64: https://code.google.com/p/zlib-win64/downloads/list
	
3.5) Python

  - 3.4.0 installation (x86 / x64)
  - debug libraries can be built from the source
  - Known bug (only in Debug): http://bugs.python.org/issue17797
  -> Workaround: pythonrun.c: replace
  
    - this: if (!is_valid_fd(fd)) {
    - with:
  
	  - #ifdef MS_WINDOWS
      - if (!is_valid_fd(fd) || GetStdHandle(STD_INPUT_HANDLE) == NULL) {
      - #else
      - if (!is_valid_fd(fd)) {
      - #endif
  - 64bit: for some reason, CMake is unable to find the library, necessary to add PATHS ${PYTHON_LIB_DIR} to the find_library(PYTHON_LIBRARY ...) command in FindPythonLibs.cmake in CMake modules directory

3.6) OpenGL
  
  - Make sure the following exists (if not, download the 8.1 Windows SDK)
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\opengl32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\glu32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\user32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\gdi32.lib

4) Before build

4.1) PATH:

  - C:\Python34;C:\Python34\Scripts
  - c:\Qt\Qt5.3.0\5.3\msvc2013_opengl\bin\ (or appropriate)
  - <PATH TO AGROS REPOSITORY>\libs\

5) Build

5.1) CMake
For MS Visual Studio 2015 (version 14):

  - cmake -G "Visual Studio 14 Win64"

5.2) Open in Visual Studio and build