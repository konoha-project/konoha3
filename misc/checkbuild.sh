#!/bin/sh
BRANCH=-`git branch --contains=HEAD | grep "*" | cut -c 3- | head -1`
DEFAULT=Debug

if [ "${BRANCH}" = "-" ]; then
    BRANCH=""
fi

for i in Debug #Compressed Release
do
(
  mkdir -p $i${BRANCH}
  cd $i${BRANCH}
  cmake ../ -DCMAKE_BUILD_TYPE=$i -DCMAKE_INSTALL_PREFIX=$HOME
  make -j8 1> /dev/null 2> $i-log || exit
)
done

make -C ${DEFAULT}${BRANCH} -j8 > /dev/null
make -C ${DEFAULT}${BRANCH} -j8 install > /dev/null
make -C ${DEFAULT}${BRANCH} test
