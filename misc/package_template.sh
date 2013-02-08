#!/bin/sh

DIR=package-devel
TYPE=$1
PACKAGE=$2

if [ "$#" -lt 2 ];then
  echo "Usage: $0 PackageType PackageName"
  exit 1
fi

NAME=${TYPE}.${PACKAGE}
HOME=`pwd`/src/${DIR}/${NAME}
SRC=${PACKAGE}_glue.c
#if [ "${DIR}" -eq "" ]; then
#    echo "usage $0 DIRNAME(package or package-devel) PKGTYPE(Syntax, Lib...) LIBNAME"
#    exit
#fi

mkdir -p ${HOME}
rm -f ${HOME}/${SRC} ${HOME}/CMakeLists.txt
echo "set(PACKAGE_SOURCE_CODE ${SRC})" >> ${HOME}/CMakeLists.txt
echo "add_konoha_package(${NAME})" >> ${HOME}/CMakeLists.txt

echo -n -e \
"/****************************************************************************\n"\
" * Copyright (c) 2013, the Konoha project authors. All rights reserved.\n"\
" * Redistribution and use in source and binary forms, with or without\n"\
" * modification, are permitted provided that the following conditions are met:\n"\
" *\n"\
" *  * Redistributions of source code must retain the above copyright notice,\n"\
" *    this list of conditions and the following disclaimer.\n"\
" *  * Redistributions in binary form must reproduce the above copyright\n"\
" *    notice, this list of conditions and the following disclaimer in the\n"\
" *    documentation and/or other materials provided with the distribution.\n"\
" *\n"\
" * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"\
" * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED\n"\
" * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"\
" * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR\n"\
" * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"\
" * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"\
" * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;\n"\
" * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"\
" * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR\n"\
" * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF\n"\
" * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"\
" ***************************************************************************/\n"\
"\n"\
"/* ************************************************************************ */\n"\
"\n"\
"#include <konoha/konoha.h>\n"\
"#include <konoha/sugar.h>\n"\
"#include <konoha/klib.h>\n"\
"#include <konoha/import/methoddecl.h>\n"\
"\n"\
"#ifdef __cplusplus\n"\
"extern \"C\" {\n"\
"#endif\n"\
"\n"\
"/* ------------------------------------------------------------------------ */\n"\
"/* ${PACKAGE} */\n"\
"\n"\
"\n"\
"// --------------------------------------------------------------------------\n"\
"static kbool_t ${PACKAGE}_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)\n"\
"{\n"\
"	return true;\n"\
"}\n"\
"\n"\
"static kbool_t ${PACKAGE}_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)\n"\
"{\n"\
"	return true;\n"\
"}\n"\
"\n"\
"KDEFINE_PACKAGE *${PACKAGE}_Init(void)\n"\
"{\n"\
"	static KDEFINE_PACKAGE d = {0};\n"\
"	KSetPackageName(d, \"${PACKAGE}\", \"0.0\");\n"\
"	d.PackupNameSpace = ${PACKAGE}_PackupNameSpace;\n"\
"	d.ExportNameSpace = ${PACKAGE}_ExportNameSpace;\n"\
"	return &d;\n"\
"}\n"\
"\n"\
"#ifdef __cplusplus\n"\
"} /* extern \"C\" */\n"\
"#endif" >> ${HOME}/${SRC}
