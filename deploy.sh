#/bin/sh

# rm files
rm -rf appimage 

# create dirs
mkdir appimage
mkdir appimage/usr
mkdir appimage/usr/bin
mkdir appimage/usr/lib
mkdir appimage/usr/share
mkdir appimage/usr/share/agrossuite

# copy binary files
cp usr/bin/agros appimage/usr/bin/
cp -r ../resources appimage/usr/share/agrossuite/resources/
cp usr/lib/libagros_*.so appimage/usr/lib/
cp usr/lib/libsolver_plugin_*.so appimage/usr/lib/
cp usr/lib/libdeal*.so* appimage/usr/lib/

# copy resources
cp -r resources_source/appimage/* appimage

# strip
strip appimage/usr/bin/*
strip appimage/usr/lib/*

# qt 
export QMAKE=/usr/bin/qmake6 
linuxdeploy-x86_64.AppImage --appdir appimage/ --executable usr/bin/agros --plugin qt --desktop-file resources_source/appimage/usr/share/applications/agros.desktop

# rm metainfo
rm -rf appimage/usr/share/metainfo

# build appimage
appimagetool-x86_64.AppImage appimage/
