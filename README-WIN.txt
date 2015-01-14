0) Dependencies
0.1) Recommended setup is to have the 32-bit dependencies in C:\hpfem\dependencies and the 64-bit ones in C:\hpfem\dependencies-64
0.2) Visual Studio 2013 Professional


1) Needed to download | build (in x86 || x64 depending on the target platform of Agros2D):

1.0) Qt

  - Qt 5.3 for the correct platform (x86 / x64), for VS 2013, with OpenGL

1.1) Xerces & XSD

  - http://www.codesynthesis.com/products/xsd/download.xhtml
  - XSD version 4.0+ necessary (for C++11 support)

1.4) ZLIB

  - x86: http://gnuwin32.sourceforge.net/packages/zlib.htm
  - x64: https://code.google.com/p/zlib-win64/downloads/list

1.5) Python

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
  - 64bit: for some reason, CMake unable to find the library, necessary to add PATHS ${PYTHON_LIB_DIR} to the find_library(PYTHON_LIBRARY ...) command in FindPythonLibs.cmake in CMake modules directory

1.6) OpenGL
  
  - Make sure the following exists (if not, download the 8.1 Windows SDK)
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\opengl32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\glu32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\user32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\gdi32.lib

2) Before build

2.1) PATH:

  - C:\Python34;C:\Python34\Scripts
  - c:\Qt\Qt5.3.0\5.3\msvc2013_opengl\bin\ (or appropriate)
  - <PATH TO AGROS REPOSITORY>\libs\
  - C:\hpfem\dependencies\bin / C:\hpfem\dependencies-64\bin


3) Build

3.1) CMake

  - x86: cmake -G "Visual Studio 12"
  - x64: cmake -G "Visual Studio 12 Win64"

3.2) Open in Visual Studio and build