#!/bin/sh
SRC="`find plugins/ -iname '*.c'` `find core/ -iname '*.c'`"
cppcheck -q --force -Iinclude -IDebug -Iplugins --enable=all ${SRC} >& cppcheck.log
scan-build -o tmp gcc -Wall ${SRC} -IDebug -Iinclude -I. -c
