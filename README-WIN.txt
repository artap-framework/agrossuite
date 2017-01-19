1) Preparing for compilation

1.1) Download and install cmake for Windows: https://cmake.org/download/
1.2) Download and install MS Visual Studio 2015 (Community Edition is enough). Don't forgot, after MSVC installation, to continue download and install necessary C++ packages (compiler and tools) - in File -> New  

2) Dependencies

??? 2.1) Compile deal.ii in Agros Suite subfolder dealii (we expect, that Agros suite project is, for example in C:\agros2d folder):
      cd C:\agros2d\dealii
      mkdir build
      cd build
      cmake -D CMAKE_INSTALL_PREFIX=C:\agros2d\dealii\install ..

2.2) Improve path to the Windows dependecies, for exaample "C:\agros_dependecies" at the beginning of the file "CMake.vars.Windows". In this case, the 32-bit dependencies in C:\agros_dependecies\32 and the 64-bit ones in C:\agros_dependecies\64

The beginning of the file "CMake.vars.Windows":
# SET dependencies directory.
# If individual dependencies are not all in this common directory, it has to be changed further down.
SET(DEPENDENCIES_DIR "C:\\agros_dependecies")
SET(DEPENDENCIES_DIR_WITH_SLASHES "C:\\agros_dependecies")

3) Needed to download | build (in x86 || x64 depending on the target platform of Agros2D):

3.0) Qt

  - Qt 5.3 for the correct platform (x86 / x64), for VS 2013, with OpenGL

3.1) Xerces & XSD

  - http://www.codesynthesis.com/products/xsd/download.xhtml
  - XSD version 4.0+ necessary (for C++11 support)

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
  - C:\hpfem\dependencies\bin / C:\hpfem\dependencies-64\bin


5) Build

5.1) CMake

  - x86: cmake -G "Visual Studio 12"
  - x64: cmake -G "Visual Studio 12 Win64"

5.2) Open in Visual Studio and build