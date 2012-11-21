#!/bin/sh
DEFAULT=Debug
for i in Debug Compressed Release
do
(
  mkdir -p $i
  cd $i
  cmake ../ -DCMAKE_BUILD_TYPE=$i -DCMAKE_INSTALL_PREFIX=$HOME
  make 1> /dev/null 2> $i-log || exit
)
done

make -C ${DEFAULT} > /dev/null
make -C ${DEFAULT} install > /dev/null
make -C ${DEFAULT} test
