#!/bin/sh

git clean -d -x -f
mkdir -p build install
cd build
cmake -G "Unix Makefiles" \
          -DCMAKE_INSTALL_PREFIX=$WORKSPACE/install \
          -DCMAKE_TOOLCHAIN_FILE=/home/masa/src/cmake/${Compiler}.cmake \
          -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
          $WORKSPACE

make
make install

ctest -T test || echo 1
xsltproc \
    /home/masa/tool/jenkins-ctest-plugin/ctest-to-junit.xsl \
    Testing/`head -n 1 < Testing/TAG`/Test.xml \
    > CTestResults.xml

