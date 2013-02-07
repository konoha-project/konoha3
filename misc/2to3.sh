#!/bin/sh

SOURCE=
for DIR in src include test test-dev sample
do
	SOURCE="${SOURCE} `find ${DIR}/ -iname "*.k"`"
done

for F in ${SOURCE}
do
	konoha -MFuelVM ./misc/2to3.k ${F}
done
