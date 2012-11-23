#!/bin/sh
SRC=`find src/ -iname "*.c"`
PKG=`find package/ -iname "*.c"`
MOD=`find module/ -iname "*.c"`

TARGET="Debug"
CFLAGS="-Iinclude -I${TARGET} -Ipackage/konoha.regexp/missing -I${TARGET}/package/posix.fd/"
cppcheck -q --force ${CFLAGS} --enable=all ${SRC} ${PKG} ${MOD}
scan-build -o /tmp/build gcc -Wall ${SRC} ${PKG} ${MOD} ${CFLAGS} -lcurl -lpcre
