#!/bin/sh

SOURCE=
for DIR in src include package
do
	SOURCE="${SOURCE} `find ${DIR}/ -iname "*.c"` `find ${DIR}/ -iname "*.h"`"
done

for F in ${SOURCE}
do
	minikonoha ./misc/CheckStyle.k ${F}
done
