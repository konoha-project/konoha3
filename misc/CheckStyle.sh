#!/bin/sh

SOURCE=
for DIR in src include
do
	for i in c h cpp; do
		SOURCE="${SOURCE} `find ${DIR}/ -iname "*.${i}"`"
	done
done

for F in ${SOURCE}
do
	minikonoha ./misc/CheckStyle.k ${F}
done
