target=Release
VERSION=3.2
LLVM=llvm-${VERSION}
BUILD=${LLVM}.src/${target}
MAKE=make
WGET=wget
BASEDIR=

.PHONY : depend
depend:
	curl -O http://llvm.org/releases/${VERSION}/llvm-${VERSION}.src.tar.gz
	tar zxf ${LLVM}.src.tar.gz
	mkdir -p ${BUILD}
	make -f `pwd`/src/module-devel/FuelVM/Makefile -C ${BUILD} build BASEDIR=`pwd`

.PHONY : build
build:
	cmake ../ -DCMAKE_BUILD_TYPE=${target} -DCMAKE_INSTALL_PREFIX=${BASEDIR}/${LLVM}
	make -f `pwd`/Makefile -C ${BASEDIR}/${BUILD}
	make -f `pwd`/Makefile -C ${BASEDIR}/${BUILD} install
	rm -fr ${BASEDIR}/${LLVM}.src.tar.gz ${BASEDIR}/${LLVM}.src
