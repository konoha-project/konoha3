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

#ifndef STRING_API_H
#define STRING_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Get the bytesize from text and number of multibyte characters.
 * e.g.) MultibyteText_bytesize("あいうえお", 3) => 9
 */
static size_t MultibyteText_bytesize(const char *text, size_t size)
{
	const unsigned char *pos = (const unsigned char *)text;
	size_t i, mindex = 0;
	for(i = 0; i < size; i++) {
		mindex += utf8len(*(pos + mindex));
	}
	return mindex;
}

/*
 * Get the number of multibyte characters from text and bytesize.
 * e.g.) MultibyteText_length("あいうえお", 9) => 3
 */
static size_t MultibyteText_length(const char *text, size_t size)
{
	size_t length = 0;
	const unsigned char *pos = (const unsigned char *)text;
	const unsigned char *end = pos + size;
	while(pos < end) {
		pos += utf8len(*pos);
		length++;
	}
	return length;
}

static int MultiByteChar_length(unsigned char s)
{
	uint8_t u = (uint8_t) s;
	assert (u >= 0x80);
	if(0xc2 <= u && u <= 0xdf)
		return 2;
	else if(0xe0 <= u && u <= 0xef)
		return 3;
	else if(0xf0 <= u && u <= 0xf4)
		return 4;
	//assert(0 && "Invalid encoding");
	return 0;
}

static size_t MultibyteString_length(KonohaContext *kctx, kString *self)
{
	size_t size = 0;
	const unsigned char *pos = (const unsigned char *)kString_text(self);
	const unsigned char *end = pos + kString_size(self);
	assert(!kString_Is(ASCII, self));
	while(pos < end) {
		size++;
		pos += utf8len(*pos);
	}
	return size;
}

#define StringPolicy_maskASCII(S)  (kString_Is(ASCII, S) ? StringPolicy_ASCII : 0)

static int String_compareTo(KonohaContext *kctx, kString *self, kString *that)
{
	size_t llen = kString_size(self);
	size_t rlen = kString_size(that);
	size_t n = (llen > rlen) ? llen : rlen;
	int ret = strncmp(kString_text(self), kString_text(that), n);
	return ret == 0 ? (intptr_t)(llen - rlen) : ret;
}

static int String_compareToIgnoreCase(KonohaContext *kctx, kString *self, kString *that)
{
	size_t llen = kString_size(self);
	size_t rlen = kString_size(that);
	size_t n = (llen > rlen) ? llen : rlen;
	int ret = strncasecmp(kString_text(self), kString_text(that), n);
	return ret == 0 ? (intptr_t)(llen - rlen) : ret;
}

static uint32_t fnv1a(const char *p, uint32_t len)
{
	uint32_t hash = 0x811c9dc5;
	const uint8_t *s = (const uint8_t *) p;
	const uint8_t *e = (const uint8_t *) p + len;
	while(s < e) {
		hash = (*s++ ^ hash) * 0x01000193;
	}
	return hash;
}

static kushort_t String_hashCode(KonohaContext *kctx, kString *self)
{
	kushort_t hash = kObject_HashCode(self);
	if(hash != 0)
		return hash;
	hash = fnv1a(kString_text(self), kString_size(self));
	kObject_SetHashCode(self, hash);
	return hash;
}

static size_t String_length(KonohaContext *kctx, kString *self)
{
	size_t bytesize = kString_size(self);
	return (kString_Is(ASCII, self)) ? bytesize : MultibyteString_length(kctx, self);
}

static uint32_t String_charAt(KonohaContext *kctx, kString *self, size_t index)
{
	const char *text = kString_text(self);
	if(kString_Is(ASCII, self)) {
		return text[index];
	}

	unsigned char *s = (unsigned char *) (text + MultibyteText_length(text, index));
	unsigned char *e = (unsigned char *) (text + kString_size(self));
	uint32_t v = 0;
	int i, length = MultiByteChar_length((unsigned char) (*s));
	if(length == 2) v = *s++ & 0x1f;
	else if(length == 3) v = *s++ & 0xf;
	else if(length == 4) v = *s++ & 0x7;
	for (i = 1; i < length && s < e; ++i) {
		uint8_t tmp = (uint8_t) *s++;
		if(tmp < 0x80 || tmp > 0xbf) {
			return 0;
		}
		v = (v << 6) | (tmp & 0x3f);
	}
	return v;
}

static bool String_startsWith(KonohaContext *kctx, kString *self, kString *prefix, size_t toffset)
{
	const char *ttext = kString_text(self) + toffset;
	const char *ptext = kString_text(prefix);
	return strncmp(ttext, ptext, kString_size(prefix)) == 0;
}

static bool String_endsWith(KonohaContext *kctx, kString *self, kString *suffix)
{
	const char *ttext = kString_text(self);
	const char *stext = kString_text(suffix);
	size_t tlen = kString_size(self);
	size_t slen = kString_size(suffix);
	return strncmp(ttext + tlen - slen, stext, slen) == 0;
}

static int String_indexOfChar(KonohaContext *kctx, kString *self, int ch, size_t fromIndex)
{
	const char *base = kString_text(self);
	const char *pos = base + fromIndex;
	const char *end = base + kString_size(self);
	while(pos < end) {
		if(*pos == ch) {
			return pos - base;
		}
		pos++;
	}
	return -1;
}

static int String_lastIndexOfChar(KonohaContext *kctx, kString *self, int ch, size_t fromIndex)
{
	const char *base = kString_text(self);
	const char *pos = base + fromIndex;
	while(pos >= base) {
		if(*pos == ch) {
			return pos - base;
		}
		pos--;
	}
	return -1;
}

static int String_indexOfString(KonohaContext *kctx, kString *self, kString *str, size_t fromIndex)
{
	const char *base = kString_text(self);
	const char *pos  = base + fromIndex;
	const char *text = kString_text(str);
	pos = strstr(pos, text);
	if(pos != NULL) {
		intptr_t loc = pos - base;
		if(!kString_Is(ASCII, self)) {
			loc = MultibyteText_length(base, (size_t)loc);
		}
		return loc;
	}
	return -1;
}

static int String_lastIndexOfString(KonohaContext *kctx, kString *self, kString *str, size_t fromIndex)
{
	const char *base = kString_text(self);
	const char *pos  = base;
	const char *text = kString_text(str);
	size_t offset = (kString_Is(ASCII, self)? fromIndex : MultibyteText_bytesize(base, fromIndex));
	pos = pos + offset;
	if(kString_size(str) == 0) {
		return kString_Is(ASCII, self)?
			fromIndex : MultibyteText_bytesize(kString_text(self), fromIndex);
	}

	intptr_t loc = -1;
	while(pos >= base) {
		if(strncmp(pos, text, kString_size(str)) == 0) {
			loc = (intptr_t)(pos - base);
			if(!kString_Is(ASCII, self)) {
				loc = MultibyteText_length(base, (size_t)(loc));
			}
			break;
		}
		pos--;
	}
	return (loc < 0) ? -1 : loc;
}

static bool String_regionMatches(KonohaContext *kctx, kString *self, bool ignoreCase, size_t toffset, kString *other, size_t ooffset, size_t len)
{
	const char *base = kString_text(self)  + toffset;
	const char *text = kString_text(other) + ooffset;
	int ret = (ignoreCase == true) ? strncasecmp(base, text, len) : strncmp(base, text, len);
	return ret == 0;
}

static kString* String_replaceFirst(KonohaContext *kctx, kString *self, kString *oldText, kString *newText)
{
	const char *text = kString_text(self);
	const char *end  = text + kString_size(self);
	const char *pos  = strstr(text, kString_text(oldText));
	const size_t oldLen = kString_size(oldText);
	if(pos == NULL || oldLen == 0)
		return self;
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_Write(kctx, &wb, text, pos - text);
	KLIB KBuffer_Write(kctx, &wb, kString_text(newText), kString_size(newText));
	KLIB KBuffer_Write(kctx, &wb, pos + oldLen, end - pos - oldLen);
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer | StringPolicy_maskASCII(self));
}

static kString* String_replace(KonohaContext *kctx, kString *self, const char *oldText, size_t oldLen, const char *newText, size_t newLen)
{
	const char *text = kString_text(self);
	const char *end  = text + kString_size(self);
	const char *pos  = strstr(text, oldText);
	if(pos == NULL || oldLen == 0)
		return self;
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_Write(kctx, &wb, text, pos - text);
	KLIB KBuffer_Write(kctx, &wb, newText, newLen);
	text = pos + oldLen;
	while((pos = strstr(text, oldText)) != NULL) {
		KLIB KBuffer_Write(kctx, &wb, text, pos - text);
		KLIB KBuffer_Write(kctx, &wb, newText, newLen);
		text = pos + oldLen;
	}
	KLIB KBuffer_Write(kctx, &wb, text, end - text);
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer | StringPolicy_maskASCII(self));
}

static kString* String_replaceChar(KonohaContext *kctx, kString *self, char oldChar, char newChar)
{
	const char oldText[2] = {oldChar, 0};
	const char newText[2] = {newChar, 0};
	return String_replace(kctx, self, oldText, 1, newText, 1);
}

static kString* String_replaceAll(KonohaContext *kctx, kString *self, kString *pattern, kString *replacement)
{
	return String_replace(kctx, self, kString_text(pattern), kString_size(pattern),
			kString_text(replacement), kString_size(replacement));
}

static kString *String_trim(KonohaContext *kctx, kString *self)
{
	const char *base = kString_text(self);
	const char *text = base;
	size_t len = kString_size(self);
	while(isspace(*text)) {
		++text;
		--len;
	}
	if(len != 0) {
		while(isspace(text[len-1])) {
			if(--len == 0)
				break;
		}
	}
	if(kString_size(self) == len) {
		return self;
	}
	return KLIB new_kString(kctx, OnStack, text, len, StringPolicy_maskASCII(self));
}

static kString* String_toupper(KonohaContext *kctx, kString *self, const char *text, const char *pos, const char *end)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_Write(kctx, &wb, text, pos - text);
	size_t len = end - pos;
	char *buf = ALLOCA(char, end - pos), *base = buf;
	while(pos < end) {
		int ch = *pos++;
		*buf++ = ('a' <= ch && ch <= 'z') ? toupper(ch) : ch;
	}
	KLIB KBuffer_Write(kctx, &wb, base, len);
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer | StringPolicy_maskASCII(self));
}


static kString* String_tolower(KonohaContext *kctx, kString *self, const char *text, const char *pos, const char *end)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_Write(kctx, &wb, text, pos - text);
	size_t len = end - pos;
	char *buf = ALLOCA(char, end - pos), *base = buf;
	while(pos < end) {
		int ch = *pos++;
		*buf++ = ('A' <= ch && ch <= 'Z') ? tolower(ch) : ch;
	}
	KLIB KBuffer_Write(kctx, &wb, base, len);
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer | StringPolicy_maskASCII(self));
}

static kString *String_toUpperCase(KonohaContext *kctx, kString *self, kString *locale)
{
	/*FIXME(ide): support locale */
	const char *base = kString_text(self);
	const char *pos  = base;
	const char *end  = base + kString_size(self);
	while(pos < end) {
		int ch = *pos;
		if('a' <= ch && ch <= 'z') {
			return String_toupper(kctx, self, base, pos, end);
		}
		pos++;
	}
	return self;
}

static kString *String_toLowerCase(KonohaContext *kctx, kString *self, kString *locale)
{
	/*FIXME(ide): support locale */
	const char *base = kString_text(self);
	const char *pos  = base;
	const char *end  = base + kString_size(self);
	while(pos < end) {
		int ch = *pos;
		if('A' <= ch && ch <= 'Z') {
			return String_tolower(kctx, self, base, pos, end);
		}
		pos++;
	}
	return self;
}

static kString *new_UTF8SubString(KonohaContext *kctx, kString *self, size_t offset, size_t length)
{
	const unsigned char *text, *pos, *end;
	size_t beginIndex, endIndex;
	assert(!kString_Is(ASCII, self));
	text       = (const unsigned char *) kString_text(self);
	beginIndex = MultibyteText_bytesize((const char *)text, offset);
	pos        = text + beginIndex;
	endIndex   = MultibyteText_bytesize((const char *)pos,  length);
	end        = (endIndex + beginIndex > kString_size(self)) ? (text + kString_size(self)) : (pos  + endIndex);
	return KLIB new_kString(kctx, OnField, (const char *)pos, end - pos, StringPolicy_UTF8);
}

static kString *String_substring(KonohaContext *kctx, kString *self, size_t beginIndex, size_t endIndex)
{
	if(kString_Is(ASCII, self)) {
		return KLIB new_kString(kctx, OnField, kString_text(self) + beginIndex, endIndex - beginIndex, StringPolicy_ASCII);
	} else {
		return new_UTF8SubString(kctx, self, beginIndex, endIndex - beginIndex);
	}
}

static kArray *String_split(KonohaContext *kctx, kArray *ret, kString *self, kString *pattern, int limit)
{
	const char *pos = kString_text(self);
	const char *end = pos + kString_size(self);
	if(kString_size(pattern) == 0) {
		size_t i, len;
		if(kString_Is(ASCII, self)) {
			len = kString_size(self);
			for(i = 0; i < len; i++) {
				KLIB kArray_Add(kctx, ret, KLIB new_kString(kctx, OnField, pos + i, 1, StringPolicy_ASCII));
			}
		}
		else {
			size_t len = MultibyteString_length(kctx, self);
			for(i = 0; i < len; i++) {
				KLIB kArray_Add(kctx, ret, new_UTF8SubString(kctx, self, i, 1));
			}
		}
	}
	else {
		size_t skip_length = kString_size(pattern);
		while(pos < end) {
			const char *res = strstr(pos, kString_text(pattern));
			if(res == NULL) {
				break;
			}
			if(res - pos > 0) {
				KLIB kArray_Add(kctx, ret, KLIB new_kString(kctx, OnField, pos, res - pos, StringPolicy_maskASCII(self)));
			}
			pos = res + skip_length;
		}
		KLIB kArray_Add(kctx, ret, KLIB new_kString(kctx, OnField, pos, end - pos, StringPolicy_maskASCII(self)));
	}
	return ret;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
