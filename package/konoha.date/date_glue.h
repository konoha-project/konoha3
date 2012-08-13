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

#ifndef DATE_GLUE_H_
#define DATE_GLUE_H_

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct _kDate kDate;
struct _kDate {
	KonohaObjectHeader h;
	struct tm *date;
};

static void Date_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kDate *d = (struct _kDate *)o;
	d->date = NULL;
}

static void Date_free(KonohaContext *kctx, kObject *o)
{
	struct _kDate *d = (struct _kDate *)o;
	if(d->date != NULL) {
		free(d->date);
		d->date = NULL;
	}
}

/* ------------------------------------------------------------------------ */
// [private functions]

/* ------------------------------------------------------------------------ */
//## Date Date.new();
static KMETHOD Date_new0(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	d->date = (struct tm *)malloc(sizeof(struct tm));
	time_t timer;
	struct tm *date;
	timer = time(NULL);
	date = localtime(&timer);
	d->date->tm_sec = date->tm_sec;
	d->date->tm_min = date->tm_min;
	d->date->tm_hour = date->tm_hour;
	d->date->tm_mday = date->tm_mday;
	d->date->tm_mon = date->tm_mon;
	d->date->tm_year = date->tm_year;
	d->date->tm_wday = date->tm_wday;
	d->date->tm_yday = date->tm_yday;
	d->date->tm_isdst = date->tm_isdst;
	fprintf(stdout, "%s", asctime(d->date));
	RETURN_((kObject *)d);
}

static KMETHOD Date_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	RETURN_((kObject *)d);
}

static KMETHOD Date_new2(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	RETURN_((kObject *)d);
}

static KMETHOD Date_new3(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	RETURN_((kObject *)d);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_Date     cDate
#define TY_Date     cDate->typeId
#define IS_Date(O)  ((O)->h.ct == CT_Date)

static	kbool_t date_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defDate = {
		STRUCTNAME(Date),
		.cflag = kClass_Final,
		.init = Date_init,
		.free = Date_free,
	};
	KonohaClass *cDate = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defDate, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Date_new0), TY_Date, TY_Date, MN_("new"), 0,
		_Public|_Const|_Im, _F(Date_new1), TY_Date, TY_Date, MN_("new"), 1, TY_Int, FN_("milliseconds"),
		_Public|_Const|_Im, _F(Date_new2), TY_Date, TY_Date, MN_("new"), 1, TY_String, FN_("dateString"),
		_Public|_Const|_Im, _F(Date_new3), TY_Date, TY_Date, MN_("new"), 7, TY_Int, FN_("year"), TY_Int, FN_("month"), TY_Int, FN_("day"), TY_Int, FN_("hour"), TY_Int, FN_("minutes"), TY_Int, FN_("seconds"), TY_Int, FN_("milliseconds"),
//		_Public|_Const|_Im, _F(Curl_setOpt), TY_void, TY_Curl, MN_("setOpt"), 2, TY_Int, FN_("type"), TY_Object/*FIXME TY_Dynamic*/, FN_("data"),
//		_Public|_Const|_Im, _F(Curl_appendHeader), TY_void, TY_Curl, MN_("appendHeader"), 1, TY_String, FN_("header"),
//		_Public|_Const|_Im, _F(Curl_perform), TY_Boolean, TY_Curl, MN_("perform"), 0,
//		_Public|_Const|_Im, _F(Curl_getInfo), TY_Object/*FIXME TY_Dynamic*/, TY_Curl, MN_("getInfo"), 1, TY_Int, FN_("type"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	return true;
}

static kbool_t date_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t date_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t date_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* DATE_GLUE_H_ */

#ifdef __cplusplus
}
#endif
