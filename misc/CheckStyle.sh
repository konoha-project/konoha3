#!/bin/sh

SOURCE=
for DIR in src include test
do
	for EXT in c h cpp
	do
		SOURCE="${SOURCE} `find ${DIR}/ -iname "*.${EXT}"`"
	done
done

for F in ${SOURCE}
do
	konoha -MFuelVM ./misc/CheckStyle.k ${F}
done
