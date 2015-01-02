#/bin/sh

rm -rf CMakeFiles
rm -f CMakeCache.txt

case "$1" in
--clang)  echo "Clang"
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
    ;;
esac

cmake -DDEAL_II_WITH_THREADS:BOOL=OFF -DDEAL_II_WITH_MUPARSER:BOOL=OFF .
