/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <stdio.h>
#include <time.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#if defined(__MINGW32__) || defined(_MSC_VER)

#define localtime_r(a, b) localtime(a)
#define gmtime_r(a, b) gmtime(a)

#ifdef _MSC_VER

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

static int gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if(NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tmpres /= 10;  /*convert into microseconds*/
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	return 0;
}

#define snprintf sprintf_s

#endif /* _MSC_VER */

#endif /* defined(__MINGW32__) || defined(_MSC_VER) */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kDateVar {
	kObjectHeader h;
	struct timeval tv;
} kDate;

/* ------------------------------------------------------------------------ */
//## Date Date.new();

static KMETHOD Date_new0(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = (struct kDateVar *)sfp[0].asDate;
	struct tm lt;
	gettimeofday((struct timeval *)&(d->tv), NULL);
	localtime_r((const time_t *)&(d->tv.tv_sec), &lt);
	KReturn((kObject *)d);
}

//## Date Date.new(int milliseconds);
static KMETHOD Date_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = (struct kDateVar *)sfp[0].asDate;
	d->tv.tv_sec = sfp[1].intValue / 1000;
	d->tv.tv_usec = sfp[1].intValue % 1000 * 1000;
	KReturn((kObject *)d);
}

//## Date Date.new(String dateString); Not implemented
//static KMETHOD Date_new2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	struct kDateVar *d = (struct kDateVar *)sfp[0].asDate;
//	KReturn((kObject *)d);
//}

//## Date Date.new(int year, int month, int day, int hours, int minutes, int seconds, int milliseconds);
static KMETHOD Date_new3(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = (struct kDateVar *)sfp[0].asDate;
	struct tm lt = {};
	if(sfp[1].intValue < 100) {
		lt.tm_year = sfp[1].intValue;
	}
	else {
		lt.tm_year = sfp[1].intValue - 1900;
	}
	lt.tm_mon  = sfp[2].intValue;
	lt.tm_mday = sfp[3].intValue;
	lt.tm_hour = sfp[4].intValue;
	lt.tm_min  = sfp[5].intValue;
	lt.tm_sec  = sfp[6].intValue;
	d->tv.tv_sec = mktime(&lt);
	d->tv.tv_usec = sfp[7].intValue * 1000;
	KReturn((kObject *)d);
}

//## int Date.getDate();
static KMETHOD Date_getDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_mday);
}

//## int Date.getDay();
static KMETHOD Date_getDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_wday);
}

//## int Date.getFullYear();
static KMETHOD Date_getFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_year + 1900);
}

//## int Date.getHours();
static KMETHOD Date_getHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_hour);
}

//## int Date.getMilliseconds();
static KMETHOD Date_getMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_usec = (sfp[0].asDate)->tv.tv_usec;
	KReturnUnboxValue((kint_t)tv_usec / 1000);
}

//## int Date.getMinutes();
static KMETHOD Date_getMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_min);
}

//## int Date.getMonth();
static KMETHOD Date_getMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_mon);
}

//## int Date.getSeconds();
static KMETHOD Date_getSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_sec);
}

//## int Date.getTime();
static KMETHOD Date_getTime(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	time_t tv_usec = (sfp[0].asDate)->tv.tv_usec;
	kint_t ret = (kint_t)tv_sec * 1000 + (kint_t)tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.getTimezoneOffset(); FIXME
static KMETHOD Date_getTimezoneOffset(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	kint_t ret = ((kint_t)utc.tm_yday * 24 * 60 + (kint_t)utc.tm_hour * 60 + (kint_t)utc.tm_min) - ((kint_t)lt.tm_yday * 24 * 60 + (kint_t)lt.tm_hour * 60 + (kint_t)lt.tm_min);
	KReturnUnboxValue(ret);
}

//## int Date.getUTCDate();
static KMETHOD Date_getUTCDate(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_mday);
}

//## int Date.getUTCDay();
static KMETHOD Date_getUTCDay(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_wday);
}

//## int Date.getUTCFullYear();
static KMETHOD Date_getUTCFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_year + 1900);
}

//## int Date.getUTCHours();
static KMETHOD Date_getUTCHours(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_hour);
}

//## int Date.getUTCMilliseconds();
static KMETHOD Date_getUTCMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_usec = (sfp[0].asDate)->tv.tv_usec;
	KReturnUnboxValue((kint_t)tv_usec / 1000);
}

//## int Date.getUTCMinutes();
static KMETHOD Date_getUTCMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_min);
}

//## int Date.getUTCMonth();
static KMETHOD Date_getUTCMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_mon);
}

//## int Date.getUTCSeconds();
static KMETHOD Date_getUTCSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	KReturnUnboxValue(utc.tm_sec);
}

//## int Date.getYear();
static KMETHOD Date_getYear(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	KReturnUnboxValue(lt.tm_year);
}

//## int Date.parse(String dateString); Not implemented
//static KMETHOD Date_Parse(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(0);
//}

//## int Date.setDate(int date);
static KMETHOD Date_SetDate(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_mday = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setFullYear(int year);
static KMETHOD Date_SetFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_year = sfp[1].intValue - 1900;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setHours(int hours);
static KMETHOD Date_SetHours(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_hour = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setMilliseconds(int milliseconds);
static KMETHOD Date_SetMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	d->tv.tv_usec = sfp[1].intValue * 1000;
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setMinutes(int minutes);
static KMETHOD Date_SetMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_min = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setMonth(int month);
static KMETHOD Date_SetMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_mon = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setSeconds(int seconds);
static KMETHOD Date_SetSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_sec = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setTime(int milliseconds);
static KMETHOD Date_SetTime(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	kint_t msec = sfp[1].intValue;
	d->tv.tv_sec = (time_t)(msec / 1000);
	d->tv.tv_usec = (time_t)(msec % 1000 * 1000);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCDate(int date);
static KMETHOD Date_SetUTCDate(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_mday = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCFullYear(int year);
static KMETHOD Date_SetUTCFullYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_year = sfp[1].intValue - 1900;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCHours(int hours);
static KMETHOD Date_SetUTCHours(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_hour = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCMilliseconds(int milliseconds);
static KMETHOD Date_SetUTCMilliseconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	d->tv.tv_usec = sfp[1].intValue * 1000;
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCMinutes(int minutes);
static KMETHOD Date_SetUTCMinutes(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_min = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCMonth(int month);
static KMETHOD Date_SetUTCMonth(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_mon = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setUTCSeconds(int seconds);
static KMETHOD Date_SetUTCSeconds(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	utc.tm_sec = sfp[1].intValue;
	d->tv.tv_sec = mktime(&utc);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

//## int Date.setYear(int year);
static KMETHOD Date_SetYear(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	lt.tm_year = sfp[1].intValue;
	d->tv.tv_sec = mktime(&lt);
	kint_t ret = (kint_t)d->tv.tv_sec * 1000 + (kint_t)d->tv.tv_usec / 1000;
	KReturnUnboxValue(ret);
}

#define MAX_STR_SIZE 64

//## String Date.toDateString();
static KMETHOD Date_toDateString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a %b %d %Y", &lt);
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

//## String Date.toGMTString();
static KMETHOD Date_toGMTString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a, %d %b %Y %H:%M:%S %Z", &utc);
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

//## String Date.toISOString();
static KMETHOD Date_toISOString(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kDateVar *d = sfp[0].asDate;
	time_t tv_sec = d->tv.tv_sec;
	struct tm utc;
	gmtime_r(&tv_sec, &utc);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%Y-%m-%dT%H:%M:%S", &utc);
	len += snprintf(str+len, MAX_STR_SIZE, ".%03dZ", (int)(d->tv.tv_usec / 1000));
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

//## Json Date.toJSON(); Not implemented
//static KMETHOD Date_toJSON(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

//## String Date.toLocaleDateString();
static KMETHOD Date_toLocaleDateString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%A %B %d %Y", &lt);
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

//## String Date.toLocaleTimeString();
static KMETHOD Date_toLocaleTimeString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%H:%M:%S", &lt);
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

//## String Date.toLocaleString();
static KMETHOD Date_toLocaleString(KonohaContext *kctx, KonohaStack *sfp)
{
	time_t tv_sec = (sfp[0].asDate)->tv.tv_sec;
	struct tm lt;
	localtime_r(&tv_sec, &lt);
	char str[MAX_STR_SIZE];
	size_t len = strftime(str, MAX_STR_SIZE, "%a %b %d %Y %H:%M:%S (%Z)", &lt);
	KReturn(KLIB new_kString(kctx, OnStack, str, len, 0));
}

/* ------------------------------------------------------------------------ */

#define KType_Date     cDate->typeId

static kbool_t date_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_CLASS defDate = {0};
	SETSTRUCTNAME(defDate, Date);
	defDate.cflag = KClassFlag_Final;

	KClass *cDate = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defDate, trace);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Date_new0), KType_Date, KType_Date, KMethodName_("new"), 0,
		_Public, _F(Date_new1), KType_Date, KType_Date, KMethodName_("new"), 1, KType_Int, KFieldName_("milliseconds"),
//		_Public, _F(Date_new2), KType_Date, KType_Date, KMethodName_("new"), 1, KType_String, KFieldName_("dateString"),
		_Public, _F(Date_new3), KType_Date, KType_Date, KMethodName_("new"), 7, KType_Int, KFieldName_("year"), KType_Int, KFieldName_("month"), KType_Int, KFieldName_("day"), KType_Int, KFieldName_("hour"), KType_Int, KFieldName_("minutes"), KType_Int, KFieldName_("seconds"), KType_Int, KFieldName_("milliseconds"),
		_Public|_Im, _F(Date_getDate), KType_Int, KType_Date, KMethodName_("getDate"), 0,
		_Public|_Im, _F(Date_getDay), KType_Int, KType_Date, KMethodName_("getDay"), 0,
		_Public|_Im, _F(Date_getFullYear), KType_Int, KType_Date, KMethodName_("getFullYear"), 0,
		_Public|_Im, _F(Date_getHours), KType_Int, KType_Date, KMethodName_("getHours"), 0,
		_Public|_Im, _F(Date_getMilliseconds), KType_Int, KType_Date, KMethodName_("getMilliseconds"), 0,
		_Public|_Im, _F(Date_getMinutes), KType_Int, KType_Date, KMethodName_("getMinutes"), 0,
		_Public|_Im, _F(Date_getMonth), KType_Int, KType_Date, KMethodName_("getMonth"), 0,
		_Public|_Im, _F(Date_getSeconds), KType_Int, KType_Date, KMethodName_("getSeconds"), 0,
		_Public|_Im, _F(Date_getTime), KType_Int, KType_Date, KMethodName_("getTime"), 0,
		_Public|_Im, _F(Date_getTimezoneOffset), KType_Int, KType_Date, KMethodName_("getTimezoneOffset"), 0,
		_Public|_Im, _F(Date_getUTCDate), KType_Int, KType_Date, KMethodName_("getUTCDate"), 0,
		_Public|_Im, _F(Date_getUTCDay), KType_Int, KType_Date, KMethodName_("getUTCDay"), 0,
		_Public|_Im, _F(Date_getUTCFullYear), KType_Int, KType_Date, KMethodName_("getUTCFullYear"), 0,
		_Public|_Im, _F(Date_getUTCHours), KType_Int, KType_Date, KMethodName_("getUTCHours"), 0,
		_Public|_Im, _F(Date_getUTCMilliseconds), KType_Int, KType_Date, KMethodName_("getUTCMilliseconds"), 0,
		_Public|_Im, _F(Date_getUTCMinutes), KType_Int, KType_Date, KMethodName_("getUTCMinutes"), 0,
		_Public|_Im, _F(Date_getUTCMonth), KType_Int, KType_Date, KMethodName_("getUTCMonth"), 0,
		_Public|_Im, _F(Date_getUTCSeconds), KType_Int, KType_Date, KMethodName_("getUTCSeconds"), 0,
		_Public|_Im, _F(Date_getYear), KType_Int, KType_Date, KMethodName_("getYear"), 0,
//		_Public|_Static, _F(Date_Parse), KType_Int, KType_Date, KMethodName_("parse"), 1, KType_String, KFieldName_("dateString"),
		_Public, _F(Date_SetDate), KType_Int, KType_Date, KMethodName_("setDate"), 1, KType_Int, KFieldName_("date"),
		_Public, _F(Date_SetFullYear), KType_Int, KType_Date, KMethodName_("setFullYear"), 1, KType_Int, KFieldName_("year"),
		_Public, _F(Date_SetHours), KType_Int, KType_Date, KMethodName_("setHours"), 1, KType_Int, KFieldName_("hours"),
		_Public, _F(Date_SetMilliseconds), KType_Int, KType_Date, KMethodName_("setMilliseconds"), 1, KType_Int, KFieldName_("milliseconds"),
		_Public, _F(Date_SetMinutes), KType_Int, KType_Date, KMethodName_("setMinutes"), 1, KType_Int, KFieldName_("minutes"),
		_Public, _F(Date_SetMonth), KType_Int, KType_Date, KMethodName_("setMonth"), 1, KType_Int, KFieldName_("month"),
		_Public, _F(Date_SetSeconds), KType_Int, KType_Date, KMethodName_("setSeconds"), 1, KType_Int, KFieldName_("seconds"),
		_Public, _F(Date_SetTime), KType_Int, KType_Date, KMethodName_("setTime"), 1, KType_Int, KFieldName_("milliseconds"),
		_Public, _F(Date_SetUTCDate), KType_Int, KType_Date, KMethodName_("setUTCDate"), 1, KType_Int, KFieldName_("date"),
		_Public, _F(Date_SetUTCFullYear), KType_Int, KType_Date, KMethodName_("setUTCFullYear"), 1, KType_Int, KFieldName_("year"),
		_Public, _F(Date_SetUTCHours), KType_Int, KType_Date, KMethodName_("setUTCHours"), 1, KType_Int, KFieldName_("hours"),
		_Public, _F(Date_SetUTCMilliseconds), KType_Int, KType_Date, KMethodName_("setUTCMilliseconds"), 1, KType_Int, KFieldName_("milliseconds"),
		_Public, _F(Date_SetUTCMinutes), KType_Int, KType_Date, KMethodName_("setUTCMinutes"), 1, KType_Int, KFieldName_("minutes"),
		_Public, _F(Date_SetUTCMonth), KType_Int, KType_Date, KMethodName_("setUTCMonth"), 1, KType_Int, KFieldName_("month"),
		_Public, _F(Date_SetUTCSeconds), KType_Int, KType_Date, KMethodName_("setUTCSeconds"), 1, KType_Int, KFieldName_("seconds"),
		_Public, _F(Date_SetYear), KType_Int, KType_Date, KMethodName_("setYear"), 1, KType_Int, KFieldName_("year"),
		_Public, _F(Date_toDateString), KType_String, KType_Date, KMethodName_("toDateString"), 0,
		_Public, _F(Date_toGMTString), KType_String, KType_Date, KMethodName_("toGMTString"), 0,
		_Public, _F(Date_toISOString), KType_String, KType_Date, KMethodName_("toISOString"), 0,
//		_Public, _F(Date_toJSON), KType_Json, KType_Date, KMethodName_("toJSON"), 0,
		_Public, _F(Date_toLocaleDateString), KType_String, KType_Date, KMethodName_("toLocaleDateString"), 0,
		_Public, _F(Date_toLocaleTimeString), KType_String, KType_Date, KMethodName_("toLocaleTimeString"), 0,
		_Public, _F(Date_toLocaleString), KType_String, KType_Date, KMethodName_("toLocaleString"), 0,
		_Public, _F(Date_toLocaleString), KType_String, KType_Date, KMethodName_("toString"), 0,
		_Public, _F(Date_toLocaleTimeString), KType_String, KType_Date, KMethodName_("toTimeString"), 0,
		_Public, _F(Date_toGMTString), KType_String, KType_Date, KMethodName_("toUTCString"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t date_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Date_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.4");
	d.PackupNameSpace = date_PackupNameSpace;
	d.ExportNameSpace = date_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
