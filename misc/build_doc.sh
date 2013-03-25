#!/bin/sh

SOURCE=
for DIR in ./${SOURCE_DIR}/src
do
	SOURCE="${SOURCE} `find ${DIR}/ -iname "ja.md"`"
done

for F in ${SOURCE}
do
	konoha misc/build_doc.k ${F} | bash -
done
