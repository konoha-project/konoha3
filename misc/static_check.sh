#!/bin/sh
SRC=`find src/ -iname "*.c"`

for EXT in c cpp
do
	SRC="${SRC} `find src/ -iname "*.${EXT}"`"
done

BRANCH=-`git branch --contains=HEAD | grep "*" | cut -c 3-`
DEFAULT=Debug
TARGET="${DEFAULT}${BRANCH}"
CFLAGS="-Iinclude -I${TARGET} -Isrc/package/JavaScript.Regexp/missing -I${TARGET}/package/posix.fd/"
cppcheck -q --force ${CFLAGS} --enable=all ${SRC}
#scan-build -o /tmp/build gcc -Wall ${SRC} ${CFLAGS} -lcurl -lpcre
