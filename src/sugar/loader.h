/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/


static kstatus_t kNameSpace_eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline);

typedef struct {
	KonohaContext *kctx;
	kNameSpace *ns;
} SugarThunk;

static int evalHookFunc(const char* script, long uline, int *isBreak, void *thunk)
{
	SugarThunk *t = (SugarThunk*)thunk;
//	if(verbose_sugar) {
//		DUMP_P("\n>>>----\n'%s'\n------\n", script);
//	}
	kstatus_t result = kNameSpace_eval(t->kctx, t->ns, script, uline);
	*isBreak = (result == K_BREAK);
	return (result != K_FAILED);
}

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static kfileline_t uline_init(KonohaContext *kctx, const char *path, int line, int isreal)
{
	kfileline_t uline = line;
	uline |= KLIB KfileId(kctx, path, strlen(path), 0, _NEWID);
	return uline;
}

static kbool_t kNameSpace_loadScript(KonohaContext *kctx, kNameSpace *ns, const char *path, kfileline_t pline)
{
	SugarThunk thunk = {kctx, ns};
	kfileline_t uline = uline_init(kctx, path, 1, true/*isRealPath*/);
	if(!(PLATAPI loadScript(path, uline, (void*)&thunk, evalHookFunc))) {
		kreportf(ErrTag, pline, "failed to load script: %s", PLATAPI shortText(path));
		return false;
	}
	return true;
}

kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, kfileline_t pline)
{
	if (KonohaContext_getSugarContext(kctx) == NULL) {
		kmodsugar->h.setup(kctx, (KonohaModule*)kmodsugar, 0/*lazy*/);
	}
	INIT_GCSTACK();
	kNameSpace *ns = new_(NameSpace, KNULL(NameSpace));
	PUSH_GCSTACK(ns);
	kstatus_t result = kNameSpace_loadScript(kctx, ns, path, pline);
	RESET_GCSTACK();
	return result;
}

// ---------------------------------------------------------------------------
// package

static kNameSpace* new_NameSpace(KonohaContext *kctx, kpackage_t packageDomain, kpackage_t packageId)
{
	kNameSpaceVar *ns = GCSAFE_new(NameSpaceVar, KNULL(NameSpace));
	ns->packageId = packageId;
	ns->packageDomain = packageId;
	return (kNameSpace*)ns;
}

static KonohaPackage *loadPackageNULL(KonohaContext *kctx, kpackage_t packageId, kfileline_t pline)
{
	const char *packageName = S_text(PackageId_s(packageId));
	char pathbuf[256];
	const char *path = PLATAPI formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_glue.k");
	KonohaPackageHandler *packageHandler = PLATAPI loadPackageHandler(packageName);
	if(path == NULL && packageHandler == NULL) {
		kreportf(ErrTag, pline, "package not found: %s path=%s", packageName, PLATAPI shortText(pathbuf));
		KLIB Kraise(kctx, EXPT_("PackageLoader"), NULL, pline);
		return NULL;
	}
	kNameSpace *ns = new_NameSpace(kctx, packageId, packageId);
	if(packageHandler != NULL && packageHandler->initPackage != NULL) {
		packageHandler->initPackage(kctx, ns, 0, NULL, pline);
	}
	if(path != NULL) {
		if(!kNameSpace_loadScript(kctx, ns, pathbuf, pline)) {
			return NULL;
		}
	}
	KonohaPackage *pack = (KonohaPackage*)KCALLOC(sizeof(KonohaPackage), 1);
	pack->packageId = packageId;
	KINITv(pack->packageNameSpace, ns);
	pack->packageHandler = packageHandler;
	path = PLATAPI formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_exports.k");
	if(path != NULL) {
		pack->exportScriptUri = KLIB KfileId(kctx, pathbuf, strlen(pathbuf), 0, _NEWID) | 1;
	}
	return pack;
}

static KonohaPackage *getPackageNULL(KonohaContext *kctx, kpackage_t packageId, kfileline_t pline)
{
	KonohaPackage *pack = (KonohaPackage*)map_getu(kctx, kmodsugar->packageMapNO, packageId, uNULL);
	isFirstTime_t flag = isFirstTime;
	if(pack == NULL) {
		pack = loadPackageNULL(kctx, packageId, pline);
		if(pack == NULL) return NULL;
		map_addu(kctx, kmodsugar->packageMapNO, packageId, (uintptr_t)pack);
		flag = Nope;
	}
	if(pack->packageHandler != NULL && pack->packageHandler->setupPackage != NULL) {
		pack->packageHandler->setupPackage(kctx, pack->packageNameSpace, flag, pline);
	}
	return pack;
}

static kbool_t kNameSpace_isImported(KonohaContext *kctx, kNameSpace *ns, kNameSpace *target, kfileline_t pline)
{
	KUtilsKeyValue* value = kNameSpace_getLocalConstNULL(kctx, ns, target->packageId | KW_PATTERN);
	if(value != NULL) {
		kreportf(DebugTag, pline, "package %s has already imported in %s", PackageId_t(ns->packageId), PackageId_t(target->packageId));
		return true;
	}
	return false;
}

static void kNameSpace_merge(KonohaContext *kctx, kNameSpace *ns, kNameSpace *target, kfileline_t pline)
{
	if(!kNameSpace_isImported(kctx, ns, target, pline)) {
		if(target->packageId != PN_konoha) {
			kNameSpace_importClassName(kctx, ns, target->packageId, pline);
		}
		if(target->constTable.bytesize > 0) {
			kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, target->constTable.keyvalueItems, target->constTable.bytesize/sizeof(KUtilsKeyValue), pline);
		}
		size_t i;
		for(i = 0; i < kArray_size(target->methodList); i++) {
			kMethod *mtd = target->methodList->methodItems[i];
			if(Method_isPublic(mtd) && mtd->packageId == target->packageId) {
				KLIB kArray_add(kctx, ns->methodList, mtd);
			}
		}
		// record imported
		kNameSpace_setConstData(kctx, ns, target->packageId | KW_PATTERN, TY_Int, target->packageId);
	}
}

static kbool_t kNameSpace_importPackage(KonohaContext *kctx, kNameSpace *ns, const char *name, kfileline_t pline)
{
	kpackage_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	KonohaPackage *pack = getPackageNULL(kctx, packageId, pline);
	if(pack != NULL) {
		kbool_t isLoadingContinue = false;
		if(ns == NULL) ns = pack->packageNameSpace;
		kNameSpace_merge(kctx, ns, pack->packageNameSpace, pline);
		if(pack->packageHandler != NULL && pack->packageHandler->initNameSpace != NULL) {
			isLoadingContinue = pack->packageHandler->initNameSpace(kctx, ns, pline);
		}
		if(isLoadingContinue && pack->exportScriptUri != 0) {
			const char *scriptPath = FileId_t(pack->exportScriptUri);
			kfileline_t uline = pack->exportScriptUri | (kfileline_t)1;
			SugarThunk thunk = {kctx, ns};
			isLoadingContinue = PLATAPI loadScript(scriptPath, uline, (void*)&thunk, evalHookFunc);
		}
		if(isLoadingContinue && pack->packageHandler != NULL && pack->packageHandler->setupNameSpace != NULL) {
			isLoadingContinue = pack->packageHandler->setupNameSpace(kctx, ns, pline);
		}
		return true;
	}
	return false;
}
