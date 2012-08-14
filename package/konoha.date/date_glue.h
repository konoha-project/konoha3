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
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct _kDate kDate;
struct _kDate {
	KonohaObjectHeader h;
	struct timeval tv;
};

/* ------------------------------------------------------------------------ */
// [private functions]

/* ------------------------------------------------------------------------ */
//## Date Date.new();
static KMETHOD Date_new0(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	struct tm lt;
	gettimeofday(&(d->tv), NULL);
	lt = *localtime(&(d->tv.tv_sec));
	fprintf(stdout, "%s", asctime(&lt));
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

//## int Date.getDate();
static KMETHOD Date_getDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_mday);
}

//## int Date.getDay();
static KMETHOD Date_getDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_wday);
}

//## int Date.getFullYear();
static KMETHOD Date_getFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_year);
}

//## int Date.getHours();
static KMETHOD Date_getHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_hour);
}

//## int Date.getMilliseconds();
static KMETHOD Date_getMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_usec = ((struct _kDate *)sfp[0].asObject)->tv.tv_usec;
	RETURNi_((kint_t)tv_usec / 1000);
}

//## int Date.getMinutes();
static KMETHOD Date_getMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_min);
}

//## int Date.getMonth();
static KMETHOD Date_getMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_mon);
}

//## int Date.getSeconds();
static KMETHOD Date_getSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	RETURNi_(lt.tm_sec);
}

//## int Date.getTime();
static KMETHOD Date_getTime(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	time_t tv_usec = ((struct _kDate *)sfp[0].asObject)->tv.tv_usec;
	kint_t ret = (kint_t)tv_sec * 1000 + (kint_t)tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.getTimezoneOffset(); FIXME
static KMETHOD Date_getTimezoneOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt = *localtime(&tv_sec);
	struct tm utc = *gmtime(&tv_sec);
	kint_t ret = ((kint_t)utc.tm_yday * 24 * 60 + (kint_t)utc.tm_hour * 60 + (kint_t)utc.tm_min) - ((kint_t)lt.tm_yday * 24 * 60 + (kint_t)lt.tm_hour * 60 + (kint_t)lt.tm_min);
	RETURNi_(ret);
}

//## int Date.getUTCDate();
static KMETHOD Date_getUTCDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_mday);
}

//## int Date.getUTCDay();
static KMETHOD Date_getUTCDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_wday);
}

//## int Date.getUTCFullYear();
static KMETHOD Date_getUTCFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_year);
}

//## int Date.getUTCHours();
static KMETHOD Date_getUTCHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_hour);
}

//## int Date.getUTCMilliseconds();
static KMETHOD Date_getUTCMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_usec = ((struct _kDate *)sfp[0].asObject)->tv.tv_usec;
	RETURNi_((kint_t)tv_usec / 1000);
}

//## int Date.getUTCMinutes();
static KMETHOD Date_getUTCMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_min);
}

//## int Date.getUTCMonth();
static KMETHOD Date_getUTCMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_mon);
}

//## int Date.getUTCSeconds();
static KMETHOD Date_getUTCSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc = *gmtime(&tv_sec);
	RETURNi_(utc.tm_sec);
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
	};
	KonohaClass *cDate = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defDate, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Date_new0), TY_Date, TY_Date, MN_("new"), 0,
		_Public|_Const|_Im, _F(Date_new1), TY_Date, TY_Date, MN_("new"), 1, TY_Int, FN_("milliseconds"),
		_Public|_Const|_Im, _F(Date_new2), TY_Date, TY_Date, MN_("new"), 1, TY_String, FN_("dateString"),
		_Public|_Const|_Im, _F(Date_new3), TY_Date, TY_Date, MN_("new"), 7, TY_Int, FN_("year"), TY_Int, FN_("month"), TY_Int, FN_("day"), TY_Int, FN_("hour"), TY_Int, FN_("minutes"), TY_Int, FN_("seconds"), TY_Int, FN_("milliseconds"),
		_Public|_Const|_Im, _F(Date_getDate), TY_Int, TY_Date, MN_("getDate"), 0,
		_Public|_Const|_Im, _F(Date_getDay), TY_Int, TY_Date, MN_("getDay"), 0,
		_Public|_Const|_Im, _F(Date_getFullYear), TY_Int, TY_Date, MN_("getFullYear"), 0,
		_Public|_Const|_Im, _F(Date_getHours), TY_Int, TY_Date, MN_("getHours"), 0,
		_Public|_Const|_Im, _F(Date_getMilliseconds), TY_Int, TY_Date, MN_("getMilliseconds"), 0,
		_Public|_Const|_Im, _F(Date_getMinutes), TY_Int, TY_Date, MN_("getMinutes"), 0,
		_Public|_Const|_Im, _F(Date_getMonth), TY_Int, TY_Date, MN_("getMonth"), 0,
		_Public|_Const|_Im, _F(Date_getSeconds), TY_Int, TY_Date, MN_("getSeconds"), 0,
		_Public|_Const|_Im, _F(Date_getTime), TY_Int, TY_Date, MN_("getTime"), 0,
		_Public|_Const|_Im, _F(Date_getTimezoneOffset), TY_Int, TY_Date, MN_("getTimezoneOffset"), 0,
		_Public|_Const|_Im, _F(Date_getUTCDate), TY_Int, TY_Date, MN_("getUTCDate"), 0,
		_Public|_Const|_Im, _F(Date_getUTCDay), TY_Int, TY_Date, MN_("getUTCDay"), 0,
		_Public|_Const|_Im, _F(Date_getUTCFullYear), TY_Int, TY_Date, MN_("getUTCFullYear"), 0,
		_Public|_Const|_Im, _F(Date_getUTCHours), TY_Int, TY_Date, MN_("getUTCHours"), 0,
		_Public|_Const|_Im, _F(Date_getUTCMilliseconds), TY_Int, TY_Date, MN_("getUTCMilliseconds"), 0,
		_Public|_Const|_Im, _F(Date_getUTCMinutes), TY_Int, TY_Date, MN_("getUTCMinutes"), 0,
		_Public|_Const|_Im, _F(Date_getUTCMonth), TY_Int, TY_Date, MN_("getUTCMonth"), 0,
		_Public|_Const|_Im, _F(Date_getUTCSeconds), TY_Int, TY_Date, MN_("getUTCSeconds"), 0,
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
