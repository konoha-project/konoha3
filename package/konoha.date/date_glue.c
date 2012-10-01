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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/float.h>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#ifdef __MINGW32__
#define localtime_r(a, b) localtime(a)
#define gmtime_r(a, b) gmtime(a)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct _kDate kDate;
struct _kDate {
	KonohaObjectHeader h;
	struct timeval tv;
};

/* ------------------------------------------------------------------------ */
//## Date Date.new();
static KMETHOD Date_new0(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	struct tm lt;
	gettimeofday(&(d->tv), NULL);
	localtime_r(&(d->tv.tv_sec), &lt);
	RETURN_((kObject *)d);
}

//## Date Date.new(int milliseconds);
static KMETHOD Date_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	d->tv.tv_sec = sfp[1].intValue / 1000;
	d->tv.tv_usec = sfp[1].intValue % 1000 * 1000;
	RETURN_((kObject *)d);
}

//## Date Date.new(String dateString); Not implemented
//static KMETHOD Date_new2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
//	RETURN_((kObject *)d);
//}

//## Date Date.new(int year, int month, int day, int hours, int minutes, int seconds, int milliseconds);
static KMETHOD Date_new3(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	struct tm lt = {};
	if(sfp[1].intValue < 100) {
		lt.tm_year =sfp[1].intValue;
	}
	else {
		lt.tm_year =sfp[1].intValue - 1900;
	}
	lt.tm_mon = sfp[2].intValue;
	lt.tm_mday = sfp[3].intValue;
	lt.tm_hour = sfp[4].intValue;
	lt.tm_min = sfp[5].intValue;
	lt.tm_sec = sfp[6].intValue;
	d->tv.tv_sec = mktime(&lt);
	d->tv.tv_usec = sfp[7].intValue * 1000;
	RETURN_((kObject *)d);
}

//## int Date.getDate();
static KMETHOD Date_getDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_mday);
}

//## int Date.getDay();
static KMETHOD Date_getDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_wday);
}

//## int Date.getFullYear();
static KMETHOD Date_getFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_year + 1900);
}

//## int Date.getHours();
static KMETHOD Date_getHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
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
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_min);
}

//## int Date.getMonth();
static KMETHOD Date_getMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_mon);
}

//## int Date.getSeconds();
static KMETHOD Date_getSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
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
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	kint_t ret = ((kint_t)utc.tm_yday * 24 * 60 + (kint_t)utc.tm_hour * 60 + (kint_t)utc.tm_min) - ((kint_t)lt.tm_yday * 24 * 60 + (kint_t)lt.tm_hour * 60 + (kint_t)lt.tm_min);
	RETURNi_(ret);
}

//## int Date.getUTCDate();
static KMETHOD Date_getUTCDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_mday);
}

//## int Date.getUTCDay();
static KMETHOD Date_getUTCDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_wday);
}

//## int Date.getUTCFullYear();
static KMETHOD Date_getUTCFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_year + 1900);
}

//## int Date.getUTCHours();
static KMETHOD Date_getUTCHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
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
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_min);
}

//## int Date.getUTCMonth();
static KMETHOD Date_getUTCMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_mon);
}

//## int Date.getUTCSeconds();
static KMETHOD Date_getUTCSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	RETURNi_(utc.tm_sec);
}

//## int Date.getYear();
static KMETHOD Date_getYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	RETURNi_(lt.tm_year);
}

//## int Date.parse(String dateString); Not implemented
//static KMETHOD Date_parse(KonohaContext *kctx, KonohaStack *sfp)
//{
//	RETURNi_(0);
//}

//## int Date.setDate(int date);
static KMETHOD Date_setDate(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_mday = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setFullYear(int year);
static KMETHOD Date_setFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_year = sfp[1].intValue - 1900;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setHours(int hours);
static KMETHOD Date_setHours(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_hour = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setMilliseconds(int milliseconds);
static KMETHOD Date_setMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	d->tv.tv_usec = sfp[1].intValue * 1000;
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setMinutes(int minutes);
static KMETHOD Date_setMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_min = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setMonth(int month);
static KMETHOD Date_setMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_mon = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setSeconds(int seconds);
static KMETHOD Date_setSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_sec = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setTime(int milliseconds);
static KMETHOD Date_setTime(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	kint_t msec = sfp[1].intValue;
	d->tv.tv_sec = (time_t)(msec / 1000);
	d->tv.tv_usec = (time_t)(msec % 1000 * 1000);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCDate(int date);
static KMETHOD Date_setUTCDate(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_mday = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCFullYear(int year);
static KMETHOD Date_setUTCFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_year = sfp[1].intValue - 1900;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCHours(int hours);
static KMETHOD Date_setUTCHours(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_hour = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCMilliseconds(int milliseconds);
static KMETHOD Date_setUTCMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	d->tv.tv_usec = sfp[1].intValue * 1000;
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCMinutes(int minutes);
static KMETHOD Date_setUTCMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_min = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCMonth(int month);
static KMETHOD Date_setUTCMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_mon = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setUTCSeconds(int seconds);
static KMETHOD Date_setUTCSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_sec = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

//## int Date.setYear(int year);
static KMETHOD Date_setYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_year = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	RETURNi_(ret);
}

#define MAX_STR_SIZE 64

//## String Date.toDateString();
static KMETHOD Date_toDateString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a %b %d %Y", &lt);
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

//## String Date.toGMTString();
static KMETHOD Date_toGMTString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a, %d %b %Y %H:%M:%S %Z", &utc);
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

//## String Date.toISOString();
static KMETHOD Date_toISOString(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kDate *d = (struct _kDate *)sfp[0].asObject;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%Y-%m-%dT%H:%M:%S", &utc);
	len += snprintf(str+len, MAX_STR_SIZE, ".%03dZ", (int)(d->tv.tv_usec / 1000));
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

//## Json Date.toJSON(); Not implemented
//static KMETHOD Date_toJSON(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

//## String Date.toLocaleDateString();
static KMETHOD Date_toLocaleDateString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%A %B %d %Y", &lt);
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

//## String Date.toLocaleTimeString();
static KMETHOD Date_toLocaleTimeString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%H:%M:%S", &lt);
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

//## String Date.toLocaleString();
static KMETHOD Date_toLocaleString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = ((struct _kDate *)sfp[0].asObject)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a %b %d %Y %H:%M:%S (%Z)", &lt);
	RETURN_(KLIB new_kString(kctx, str, len, 0));
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im       kMethod_Immutable
#define _Static   kMethod_Static
#define _F(F)   (intptr_t)(F)

#define CT_Date     cDate
#define TY_Date     cDate->typeId
#define IS_Date(O)  ((O)->h.ct == CT_Date)

static kbool_t date_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defDate = {
		STRUCTNAME(Date),
		.cflag = kClass_Final,
	};
	KonohaClass *cDate = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defDate, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Date_new0), TY_Date, TY_Date, MN_("new"), 0,
		_Public, _F(Date_new1), TY_Date, TY_Date, MN_("new"), 1, TY_int, FN_("milliseconds"),
//		_Public, _F(Date_new2), TY_Date, TY_Date, MN_("new"), 1, TY_String, FN_("dateString"),
		_Public, _F(Date_new3), TY_Date, TY_Date, MN_("new"), 7, TY_int, FN_("year"), TY_int, FN_("month"), TY_int, FN_("day"), TY_int, FN_("hour"), TY_int, FN_("minutes"), TY_int, FN_("seconds"), TY_int, FN_("milliseconds"),
		_Public|_Im, _F(Date_getDate), TY_int, TY_Date, MN_("getDate"), 0,
		_Public|_Im, _F(Date_getDay), TY_int, TY_Date, MN_("getDay"), 0,
		_Public|_Im, _F(Date_getFullYear), TY_int, TY_Date, MN_("getFullYear"), 0,
		_Public|_Im, _F(Date_getHours), TY_int, TY_Date, MN_("getHours"), 0,
		_Public|_Im, _F(Date_getMilliseconds), TY_int, TY_Date, MN_("getMilliseconds"), 0,
		_Public|_Im, _F(Date_getMinutes), TY_int, TY_Date, MN_("getMinutes"), 0,
		_Public|_Im, _F(Date_getMonth), TY_int, TY_Date, MN_("getMonth"), 0,
		_Public|_Im, _F(Date_getSeconds), TY_int, TY_Date, MN_("getSeconds"), 0,
		_Public|_Im, _F(Date_getTime), TY_int, TY_Date, MN_("getTime"), 0,
		_Public|_Im, _F(Date_getTimezoneOffset), TY_int, TY_Date, MN_("getTimezoneOffset"), 0,
		_Public|_Im, _F(Date_getUTCDate), TY_int, TY_Date, MN_("getUTCDate"), 0,
		_Public|_Im, _F(Date_getUTCDay), TY_int, TY_Date, MN_("getUTCDay"), 0,
		_Public|_Im, _F(Date_getUTCFullYear), TY_int, TY_Date, MN_("getUTCFullYear"), 0,
		_Public|_Im, _F(Date_getUTCHours), TY_int, TY_Date, MN_("getUTCHours"), 0,
		_Public|_Im, _F(Date_getUTCMilliseconds), TY_int, TY_Date, MN_("getUTCMilliseconds"), 0,
		_Public|_Im, _F(Date_getUTCMinutes), TY_int, TY_Date, MN_("getUTCMinutes"), 0,
		_Public|_Im, _F(Date_getUTCMonth), TY_int, TY_Date, MN_("getUTCMonth"), 0,
		_Public|_Im, _F(Date_getUTCSeconds), TY_int, TY_Date, MN_("getUTCSeconds"), 0,
		_Public|_Im, _F(Date_getYear), TY_int, TY_Date, MN_("getYear"), 0,
//		_Public|_Static, _F(Date_parse), TY_int, TY_Date, MN_("parse"), 1, TY_String, FN_("dateString"),
		_Public, _F(Date_setDate), TY_int, TY_Date, MN_("setDate"), 1, TY_int, FN_("date"),
		_Public, _F(Date_setFullYear), TY_int, TY_Date, MN_("setFullYear"), 1, TY_int, FN_("year"),
		_Public, _F(Date_setHours), TY_int, TY_Date, MN_("setHours"), 1, TY_int, FN_("hours"),
		_Public, _F(Date_setMilliseconds), TY_int, TY_Date, MN_("setMilliseconds"), 1, TY_int, FN_("milliseconds"),
		_Public, _F(Date_setMinutes), TY_int, TY_Date, MN_("setMinutes"), 1, TY_int, FN_("minutes"),
		_Public, _F(Date_setMonth), TY_int, TY_Date, MN_("setMonth"), 1, TY_int, FN_("month"),
		_Public, _F(Date_setSeconds), TY_int, TY_Date, MN_("setSeconds"), 1, TY_int, FN_("seconds"),
		_Public, _F(Date_setTime), TY_int, TY_Date, MN_("setTime"), 1, TY_int, FN_("milliseconds"),
		_Public, _F(Date_setUTCDate), TY_int, TY_Date, MN_("setUTCDate"), 1, TY_int, FN_("date"),
		_Public, _F(Date_setUTCFullYear), TY_int, TY_Date, MN_("setUTCFullYear"), 1, TY_int, FN_("year"),
		_Public, _F(Date_setUTCHours), TY_int, TY_Date, MN_("setUTCHours"), 1, TY_int, FN_("hours"),
		_Public, _F(Date_setUTCMilliseconds), TY_int, TY_Date, MN_("setUTCMilliseconds"), 1, TY_int, FN_("milliseconds"),
		_Public, _F(Date_setUTCMinutes), TY_int, TY_Date, MN_("setUTCMinutes"), 1, TY_int, FN_("minutes"),
		_Public, _F(Date_setUTCMonth), TY_int, TY_Date, MN_("setUTCMonth"), 1, TY_int, FN_("month"),
		_Public, _F(Date_setUTCSeconds), TY_int, TY_Date, MN_("setUTCSeconds"), 1, TY_int, FN_("seconds"),
		_Public, _F(Date_setYear), TY_int, TY_Date, MN_("setYear"), 1, TY_int, FN_("year"),
		_Public, _F(Date_toDateString), TY_String, TY_Date, MN_("toDateString"), 0,
		_Public, _F(Date_toGMTString), TY_String, TY_Date, MN_("toGMTString"), 0,
		_Public, _F(Date_toISOString), TY_String, TY_Date, MN_("toISOString"), 0,
//		_Public, _F(Date_toJSON), TY_Json, TY_Date, MN_("toJSON"), 0,
		_Public, _F(Date_toLocaleDateString), TY_String, TY_Date, MN_("toLocaleDateString"), 0,
		_Public, _F(Date_toLocaleTimeString), TY_String, TY_Date, MN_("toLocaleTimeString"), 0,
		_Public, _F(Date_toLocaleString), TY_String, TY_Date, MN_("toLocaleString"), 0,
		_Public, _F(Date_toLocaleString), TY_String, TY_Date, MN_("toString"), 0,
		_Public, _F(Date_toLocaleTimeString), TY_String, TY_Date, MN_("toTimeString"), 0,
		_Public, _F(Date_toGMTString), TY_String, TY_Date, MN_("toUTCString"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	return true;
}

static kbool_t date_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t date_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t date_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* date_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "date", "1.0");
	d.initPackage    = date_initPackage;
	d.setupPackage   = date_setupPackage;
	d.initNameSpace  = date_initNameSpace;
	d.setupNameSpace = date_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
