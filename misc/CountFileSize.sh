#!/bin/sh
SOURCE=
for DIR in src/konoha src/parser include src/module/ExecutionEngine/MiniVM src/module/GC/BitmapGC src/exec/command
do
	for EXT in c h cpp
	do
		SOURCE="${SOURCE} `find ${DIR}/ -iname "*.${EXT}"`"
	done
done

wc -l ${SOURCE}

BUILD_DIR=Debug-`git branch --contains=HEAD | grep "*" | cut -c 3-`
EXT=so
echo "------------------------------"
echo "Library FileSize"
du -k ${BUILD_DIR}/libkonoha.${EXT}
