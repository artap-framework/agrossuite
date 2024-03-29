#/bin/sh

# init cache
export PATH="/usr/lib/ccache:$PATH"
export CCACHE_BASEDIR="$PWD"
export CCACHE_DIR="$PWD/ccache"
export CCACHE_COMPILERCHECK=content
ccache --zero-stats || true
ccache --show-stats || true

# build
cd dealii
rm -rf build
mkdir -p build 
cd build 
cmake -DCMAKE_BUILD_TYPE="Release" -DDEAL_II_WITH_ZLIB="ON" -DDEAL_II_WITH_UMFPACK="OFF" -DDEAL_II_FORCE_BUNDLED_BOOST="ON" -DDEAL_II_WITH_TBB="ON" -DDEAL_II_WITH_GMSH="OFF" -DDEAL_II_WITH_MUPARSER="OFF" -DDEAL_II_WITH_ARPACK="OFF" -DDEAL_II_COMPONENT_EXAMPLES="OFF" ..
make -j12
cd ../..
# copy deal
cp -f dealii/build/lib/libdeal_II.so.9.5.2 libs/libdeal_II.so.9.5.2

# cache stats
ccache --show-stats 
