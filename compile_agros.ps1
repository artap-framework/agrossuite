# Klonování repozitáře
git clone git@gitlab.fel.zcu.cz:agros/agros2d.git
Set-Location .\agros2d

# Inicializace a aktualizace podmodulů
git submodule init
git submodule update

# Kopírování boost knihovny
xcopy "C:\agros_dependencies\source\boost_1_70_0\boost_1_70_0\boost\ptr_container" "C:\Users\vboxuser\Documents\Release\agros2d\dealii\bundled\boost-1.70.0\include\boost\ptr_container\" /s /e

# Příprava dealii
Set-Location .\dealii
mkdir build
Set-Location .\build

# CMake konfigurace a sestavení
cmake -DCMAKE_BUILD_TYPE="Release" -DDEAL_II_WITH_ZLIB="ON" -DDEAL_II_WITH_UMFPACK="OFF" -DDEAL_II_FORCE_BUNDLED_BOOST="ON" -DDEAL_II_WITH_TBB="OFF" -DDEAL_II_WITH_GMSH="OFF" -DDEAL_II_WITH_MUPARSER="OFF" -DDEAL_II_WITH_ARPACK="OFF" -DDEAL_II_COMPONENT_EXAMPLES="OFF" -DZLIB_INCLUDE_DIR="C:/agros_dependencies/source/zlib131/zlib-1.3.1" -DZLIB_LIBRARY="C:/agros_dependencies/source/zlib131/zlib-1.3.1/Release/zlib.lib" ..
cmake --build . --config Release

# Návrat do nadřazeného adresáře
Set-Location ..
Set-Location ..

# Sestavení agros
cmake .
cmake --build . --config Release

# Spuštění agros_generator.exe
.\agros_generator.exe

# Sestavení pluginů
Set-Location .\plugins
cmake .
cmake --build . --config Release

# Kopírování deal_II.lib do složky s knihovnami
Copy-Item "C:\Users\vboxuser\Documents\Release\agros2d\dealii\build\lib\deal_II.lib" "C:\Users\vboxuser\Documents\Release\agros2d\libs"

# Příprava prostředí pro Python
Set-Location ..\agros-python
python -m venv env
Set-Location .\env\Scripts
.\activate

# Instalace Python balíčků
pip install cython
pip install scikit-build
pip install pytest
pip install numpy
pip install agrossuite/.
Set-Location ..
Set-Location ..

# Kopírování souborů
Copy-Item "C:\Users\vboxuser\Documents\Release\install\Qt6Core.dll" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"
Copy-Item "C:\Users\vboxuser\Documents\Release\install\Qt6Core5Compat.dll" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"
Copy-Item "C:\Users\vboxuser\Documents\Release\agros2d\*.dll" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"
Copy-Item "C:\Users\vboxuser\Documents\Release\install\triangle.exe" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"
Copy-Item "C:\Users\vboxuser\Documents\Release\install\gmsh.exe" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"
Copy-Item "C:\Users\vboxuser\Documents\Release\install\zlib.dll" "C:\Users\vboxuser\Documents\Release\agros2d\agros-python\env\Lib\site-packages\agrossuite"

# Spuštění testů
Set-Location .\tests
pytest --junitxml=../../report.xml --disable-warnings --durations=0 -v
Set-Location ..
Set-Location ..

# Spuštění kompilace
Compil32.exe /cc agros.iss
