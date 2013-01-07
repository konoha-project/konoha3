#!/bin/sh

SOURCE=
for DIR in src include
do
	for EXT in c h cpp
	do
		SOURCE="${SOURCE} `find ${DIR}/ -iname "*.${EXT}"`"
	done
done

for F in ${SOURCE}
do
	minikonoha ./misc/CheckStyle.k ${F}
done
