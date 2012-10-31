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

#ifndef JAVA_STRING_H
#define JAVA_STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#define To_char(sfp)    ((char)sfp.intValue)
#define To_Locale(sfp)  sfp.asString
typedef kString *String;
typedef kArray  *Array;
typedef String Locale;
typedef bool boolean;
#define RETURNchar(RET)    KReturnInt(RET)
#define RETURNint(RET)     KReturnInt(RET)
#define RETURNboolean(RET) KReturnUnboxValue(RET)
#define RETURNString(RET)  KReturn(RET)
#define RETURNArray(RET)   KReturn(RET)
#define TYPE_Array(TYPE)      (CT_p0(kctx, CT_Array, TY_##TYPE))->typeId
#define TY_char            TY_int
#define TY_Locale          TY_String

static void Throw_IndexOutOfBoundsException(KonohaContext *kctx, KonohaStack *sfp, intptr_t index)
{
	KMakeTrace(trace, sfp);
	KTraceApi(trace, UserFault, "charAt",
			LogUint("index", index));
	KLIB KonohaRuntime_raise(kctx, EXPT_("IndexOutOfBounds"),
			UserFault, NULL, trace->baseStack);
}

static void Throw_NotImplement(KonohaContext *kctx, KonohaStack *sfp, const char *FuncName)
{
	KMakeTrace(trace, sfp);
	KTraceApi(trace, UserFault, "charAt",
			LogText("Func", FuncName));
	KLIB KonohaRuntime_raise(kctx, EXPT_("NotImplement"),
			SystemFault, NULL, trace->baseStack);
}

// Returns the char value at the specified index.
//##["@Public", "char", "String_charAt", "int index"]
static KMETHOD KString_charAt(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t index = sfp[1].intValue;
	if(index > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	unsigned char ret = String_charAt(kctx, self, index);
	RETURNchar(ret);
}

// Returns the character
//##["@Public", "int", "String_codePointAt", "int index", "@Returns the character "]
static KMETHOD KString_codePointAt(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t index = sfp[1].intValue;
	if(index > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	Throw_NotImplement(kctx, sfp, __FUNCTION__);
	//int ret = String_codePointAt(self, index);
	//RETURNint(ret);
}

// Returns the character
//##["@Public", "int", "String_codePointBefore", "int index", "@Returns the character "]
static KMETHOD KString_codePointBefore(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t index = sfp[1].intValue;
	if(index > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	Throw_NotImplement(kctx, sfp, __FUNCTION__);
	//int ret = String_codePointBefore(self, index);
	//RETURNint(ret);
}

// Returns the number of Unicode code points in the specified text range of this String_
//##["@Public", "int", "String_codePointCount", "int beginIndex", " int endIndex"]
static KMETHOD KString_codePointCount(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t beginIndex = sfp[1].intValue;
	size_t endIndex = sfp[2].intValue;
	if(beginIndex > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	if(beginIndex > endIndex || endIndex > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[2].intValue);
	}
	Throw_NotImplement(kctx, sfp, __FUNCTION__);
	//int ret = String_codePointCount(self, beginIndex, endIndex);
	//RETURNint(ret);
}

// Compares two strings lexicographically.
//##["@Public", "int", "String_compareTo", "String anotherString"]
static KMETHOD KString_compareTo(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String anotherString = sfp[1].asString;
	int ret = String_compareTo(kctx, self, anotherString);
	RETURNint(ret);
}

// Compares two strings lexicographically
//##["@Public", "int", "String_compareToIgnoreCase", "String str", "@Compares two strings lexicographically"]
static KMETHOD KString_compareToIgnoreCase(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	kString *str = sfp[1].asString;
	int ret = String_compareToIgnoreCase(kctx, self, str);
	RETURNint(ret);
}

// Tests if this string ends with the specified suffix.
//##["@Public", "boolean", "String_endsWith", "String suffix"]
static KMETHOD KString_endsWith(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String suffix = sfp[1].asString;
	boolean ret = String_endsWith(kctx, self, suffix);
	RETURNboolean(ret);
}

// Returns a hash code for this String_
//##["@Public", "int", "String_hashCode"]
static KMETHOD KString_hashCode(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	int ret = String_hashCode(kctx, self);
	RETURNint(ret);
}

// Returns the index within this string of the first occurrence of the specified character
//##["@Public", "int", "String_indexOf", "int ch", " int fromIndex", "@Returns the index within this string of the first occurrence of the specified character"]
static KMETHOD KString_indexOf0(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	int ch = sfp[1].intValue;
	size_t fromIndex = sfp[2].intValue;
	int ret = (fromIndex < S_size(self)) ? String_indexOfChar(kctx, self, ch, fromIndex) : -1;
	RETURNint(ret);
}

// Returns the index within this string of the first occurrence of the specified substring
//##["@Public", "int", "String_indexOf", "String str", " int fromIndex", "@Returns the index within this string of the first occurrence of the specified substring"]
static KMETHOD KString_indexOf1(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	kString *str = sfp[1].asString;
	size_t fromIndex = sfp[2].intValue;
	int ret = (S_size(str) != 0) ?
		(fromIndex < S_size(self)) ? String_indexOfString(kctx, self, str, fromIndex) : -1 :
		(int)fromIndex;
	RETURNint(ret);
}

// Returns the index within this string of the last occurrence of the specified character
//##["@Public", "int", "String_lastIndexOf", "int ch", " int fromIndex", "@Returns the index within this string of the last occurrence of the specified character"]
static KMETHOD KString_lastIndexOf0(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	int ch = sfp[1].intValue;
	int fromIndex = sfp[2].intValue;
	int ret = String_lastIndexOfChar(kctx, self, ch, fromIndex);
	RETURNint(ret);
}

// Returns the index within this string of the last occurrence of the specified substring
//##["@Public", "int", "String_lastIndexOf", "String str", " int fromIndex", "@Returns the index within this string of the last occurrence of the specified substring"]
static KMETHOD KString_lastIndexOf1(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	kString *str = sfp[1].asString;
	int fromIndex = sfp[2].intValue;
	int ret = String_lastIndexOfString(kctx, self, str, fromIndex);
	RETURNint(ret);
}

// Returns the length of this String_
//##["@Public", "int", "String_length"]
static KMETHOD KString_length(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	int ret = String_length(kctx, self);
	RETURNint(ret);
}

// Tells whether or not this string matches the given regular expression.
//##["@Public", "boolean", "String_matches", "String regex"]
static KMETHOD KString_matches(KonohaContext *kctx,  KonohaStack *sfp)
{
	//kString *self = sfp[0].asString;
	//String regex = sfp[1].asString;
	//boolean ret = String_matches(self, regex);
	//RETURNboolean(ret);
	/* FIXME:this function need to implement regular expressions. */
	Throw_NotImplement(kctx, sfp, __FUNCTION__);
}

// Returns the index within this String that is offset from the given index by codePointOffset code points.
//##["@Public", "int", "String_offsetByCodePoints", "int index", " int codePointOffset"]
static KMETHOD KString_offsetByCodePoints(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t index = sfp[1].intValue;
	//int codePointOffset = sfp[2].intValue;
	if(index > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	Throw_NotImplement(kctx, sfp, __FUNCTION__);
	//int ret = String_offsetByCodePoints(self, index, codePointOffset);
	//RETURNint(ret);
}

// Tests if two string regions are equal.
//##["@Public", "boolean", "String_regionMatches", "boolean ignoreCase", " int toffset", " String other", " int ooffset", " int len"]
static KMETHOD KString_regionMatches(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	boolean ignoreCase = sfp[1].boolValue;
	intptr_t toffset = sfp[2].intValue;
	String other = sfp[3].asString;
	intptr_t ooffset = sfp[4].intValue;
	int len = sfp[5].intValue;
	if(toffset < 0 || ooffset < 0 || len < 0) {
		RETURNboolean(false);
	}
	if((size_t)toffset + len > S_size(self)) {
		RETURNboolean(false);
	}
	if((size_t)ooffset + len > S_size(other)) {
		RETURNboolean(false);
	}
	boolean ret = String_regionMatches(kctx, self, ignoreCase, toffset, other, ooffset, len);
	RETURNboolean(ret);
}

// Returns a new string resulting from replacing all occurrences of oldChar in this string with newChar.
//##["@Public", "String", "String_replace", "char oldChar", " char newChar"]
static KMETHOD KString_replace(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	char oldChar = To_char(sfp[1]);
	char newChar = To_char(sfp[2]);
	String ret = String_replaceChar(kctx, self, oldChar, newChar);
	RETURNString(ret);
}

// Replaces each substring of this string that matches the given regular expression with the given replacement.
//##["@Public", "String", "String_replaceAll", "String regex", " String replacement"]
static KMETHOD KString_replaceAll(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String regex = sfp[1].asString;
	String replacement = sfp[2].asString;
	String ret = String_replaceAll(kctx, self, regex, replacement);
	RETURNString(ret);
}

// Replaces the first substring of this string that matches the given regular expression with the given replacement.
//##["@Public", "String", "String_replaceFirst", "String regex", " String replacement"]
static KMETHOD KString_replaceFirst(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String regex = sfp[1].asString;
	String replacement = sfp[2].asString;
	String ret = String_replaceFirst(kctx, self, regex, replacement);
	RETURNString(ret);
}

// Splits this string around matches of the given regular expression.
//##["@Public", "Array(String)", "String_split", "String regex", " int limit"]
static KMETHOD KString_split(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String regex = sfp[1].asString;
	int limit = sfp[2].intValue;
	kArray *ret = (kArray *) KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	ret = String_split(kctx, ret, self, regex, limit);
	RETURNArray(ret);
}

// Tests if the substring of this string beginning at the specified index starts with the specified prefix.
//##["@Public", "boolean", "String_startsWith", "String prefix", " int toffset"]
static KMETHOD KString_startsWith(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String prefix = sfp[1].asString;
	size_t toffset = sfp[2].intValue;
	bool ret = (toffset <= S_size(self)) ? String_startsWith(kctx, self, prefix, toffset) : false;
	RETURNboolean(ret);
}

// Returns a new string that is a substring of this String_
//##["@Public", "String", "String_substring", "int beginIndex", " int endIndex"]
static KMETHOD KString_substring(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t beginIndex = sfp[1].intValue;
	size_t endIndex = sfp[2].intValue;
	if(beginIndex > S_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	if(endIndex > S_size(self) || beginIndex > endIndex) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[2].intValue);
	}
	String ret = String_substring(kctx, self, beginIndex, endIndex);
	RETURNString(ret);
}

// Converts all of the characters in this String to lower case using the rules of the given Locale.
//##["@Public", "String", "String_toLowerCase", "Locale locale"]
static KMETHOD KString_toLowerCase(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	Locale locale = To_Locale(sfp[1]);
	String ret = String_toLowerCase(kctx, self, locale);
	RETURNString(ret);
}

// Converts all of the characters in this String to upper case using the rules of the given Locale.
//##["@Public", "String", "String_toUpperCase", "Locale locale"]
static KMETHOD KString_toUpperCase(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	Locale locale = To_Locale(sfp[1]);
	String ret = String_toUpperCase(kctx, self, locale);
	RETURNString(ret);
}

// Returns a copy of the string
//##["@Public", "String", "String_trim", "@Returns a copy of the string"]
static KMETHOD KString_trim(KonohaContext *kctx,  KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	String ret = String_trim(kctx, self);
	RETURNString(ret);
}

static kbool_t LoadJavaAPI(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		//_Public, _F(KString_charAt), TY_char, TY_String, MN_("charAt"), 1, TY_int, FN_("index"),
		_Public|_Const|_Im, _F(KString_codePointAt), TY_int, TY_String, MN_("codePointAt"), 1, TY_int, FN_("index"),
		_Public|_Const|_Im, _F(KString_codePointBefore), TY_int, TY_String, MN_("codePointBefore"), 1, TY_int, FN_("index"),
		_Public|_Const|_Im, _F(KString_codePointCount), TY_int, TY_String, MN_("codePointCount"), 2, TY_int, FN_("beginIndex"),TY_int, FN_("endIndex"),
		_Public|_Const|_Im, _F(KString_compareTo), TY_int, TY_String, MN_("compareTo"), 1, TY_String, FN_("anotherString"),
		_Public|_Const|_Im, _F(KString_compareToIgnoreCase), TY_int, TY_String, MN_("compareToIgnoreCase"), 1, TY_String, FN_("str"),
		_Public|_Const|_Im, _F(KString_endsWith), TY_boolean, TY_String, MN_("endsWith"), 1, TY_String, FN_("suffix"),
		_Public|_Const    , _F(KString_hashCode), TY_int, TY_String, MN_("hashCode"), 0,
		_Public|_Const|_Im, _F(KString_indexOf0), TY_int, TY_String, MN_("indexOf"), 2, TY_int, FN_("ch"),TY_int, FN_("fromIndex"),
		_Public|_Const|_Im, _F(KString_indexOf1), TY_int, TY_String, MN_("indexOf"), 2, TY_String, FN_("str"),TY_int, FN_("fromIndex"),
		_Public|_Const|_Im, _F(KString_lastIndexOf0), TY_int, TY_String, MN_("lastIndexOf"), 2, TY_int, FN_("ch"),TY_int, FN_("fromIndex"),
		_Public|_Const|_Im, _F(KString_lastIndexOf1), TY_int, TY_String, MN_("lastIndexOf"), 2, TY_String, FN_("str"),TY_int, FN_("fromIndex"),
		_Public|_Const|_Im, _F(KString_length), TY_int, TY_String, MN_("length"), 0,
		_Public|_Const|_Im, _F(KString_matches), TY_boolean, TY_String, MN_("matches"), 1, TY_String, FN_("regex"),
		_Public|_Const|_Im, _F(KString_offsetByCodePoints), TY_int, TY_String, MN_("offsetByCodePoints"), 2, TY_int, FN_("index"),TY_int, FN_("codePointOffset"),
		_Public|_Const|_Im, _F(KString_regionMatches), TY_boolean, TY_String, MN_("regionMaches"), 5, TY_boolean, FN_("ignoreCase"),TY_int, FN_("toffset"),TY_String, FN_("other"),TY_int, FN_("ooffset"),TY_int, FN_("len"),
		_Public|_Const|_Im, _F(KString_replace), TY_String, TY_String, MN_("replace"), 2, TY_char, FN_("oldChar"),TY_char, FN_("newChar"),
		_Public|_Const|_Im, _F(KString_replaceAll), TY_String, TY_String, MN_("replaceAll"), 2, TY_String, FN_("regex"),TY_String, FN_("replacement"),
		_Public|_Const|_Im, _F(KString_replaceFirst), TY_String, TY_String, MN_("replaceFirst"), 2, TY_String, FN_("regex"),TY_String, FN_("replacement"),
		_Public|_Const|_Im, _F(KString_split), TYPE_Array(String), TY_String, MN_("split"), 2, TY_String, FN_("regex"),TY_int, FN_("limit"),
		_Public|_Const|_Im, _F(KString_startsWith), TY_boolean, TY_String, MN_("startsWith"), 2, TY_String, FN_("prefix"),TY_int, FN_("toffset"),
		_Public|_Const|_Im, _F(KString_substring), TY_String, TY_String, MN_("substring"), 2, TY_int, FN_("beginIndex"),TY_int, FN_("endIndex"),
		_Public|_Const|_Im, _F(KString_toLowerCase), TY_String, TY_String, MN_("toLowerCase"), 1, TY_Locale, FN_("locale"),
		_Public|_Const|_Im, _F(KString_toUpperCase), TY_String, TY_String, MN_("toUpperCase"), 1, TY_Locale, FN_("locale"),
		_Public|_Const|_Im, _F(KString_trim), TY_String, TY_String, MN_("trim"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
