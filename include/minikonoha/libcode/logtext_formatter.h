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

#ifndef LOGFORMATTER_H_
#define LOGFORMATTER_H_

// --------------------------------------------------------------------------

#define writeToBuffer(CH, buftop, bufend) { buftop[0] = CH; buftop++; }

#define TEXTSIZE(T)   T, (sizeof(T) - 1)

static char *writeFixedTextToBuffer(const char *text, size_t len, char *buftop, char *bufend)
{
	if(bufend - buftop > len) {
		memcpy(buftop, text, len);
		return buftop+len;
	}
	return buftop;
}

static char *writeTextToBuffer(const char *s, char *buftop, char *bufend)
{
	if(buftop < bufend) {
		buftop[0] = '"';
		buftop++;
	}
	while(*s != 0 && buftop < bufend) {
		if(*s == '"') {
			buftop[0] = '\"'; buftop++;
			if(buftop < bufend) {
				buftop[0] = s[0];
				buftop++;
			}
		}
		else if(*s == '\n') {
			buftop[0] = '\\'; buftop++;
			if(buftop < bufend) {
				buftop[0] = 'n';
				buftop++;
			}
		}
		else {
			buftop[0] = s[0];
			buftop++;
		}
		s++;
	}
	if(buftop < bufend) {
		buftop[0] = '"';
		buftop++;
	}
	return buftop;
}

static void reverse(char *const start, char *const end, const int len)
{
	int i, l = len / 2;
	register char *s = start;
	register char *e = end - 1;
	for (i = 0; i < l; i++) {
		char tmp = *s;
		*s++ = *e;
		*e-- = tmp;
	}
}

static char *writeUnsingedIntToBuffer(uintptr_t uint, char *const buftop, const char *const bufend)
{
	int i = 0;
	while (buftop + i < bufend) {
		int tmp = uint % 10;
		uint /= 10;
		buftop[i] = '0' + tmp;
		++i;
		if (uint == 0)
			break;
	}
	reverse(buftop, buftop + i, i);
	return buftop + i;
}

static char* writeKeyToBuffer(const char *key, size_t keylen, char *buftop, char *bufend)
{
	if(buftop < bufend) {
		writeToBuffer('"', buftop, bufend);
	}
	buftop = writeFixedTextToBuffer(key, keylen, buftop, bufend);
	if(buftop + 3 < bufend) {
		buftop[0] = '"';
		buftop[1] = ':';
		buftop[2] = ' ';
		buftop+=3;
	}
	return buftop;
}

#define HasFault    (SystemFault|ScriptFault|DataFault|ExternalFault)
#define HasLocation (PeriodicPoint|PreactionPoint|ActionPoint|SecurityAudit)

static char* writePolicyToBuffer(logconf_t *logconf, char *buftop, char *bufend)
{
	if((logconf->policy & HasLocation)) {
		buftop = writeKeyToBuffer(TEXTSIZE("TracePoint"), buftop, bufend);
		writeToBuffer('"', buftop, bufend);
		if(TFLAG_is(int, logconf->policy, PeriodicPoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("Periodic,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, PreactionPoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("PreAction,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, ActionPoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("Action,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, SecurityAudit)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SecurityAudit,"), buftop, bufend);
		}
		buftop[-1] = '"';
		buftop[0] = ',';
		buftop[1] = ' ';
		buftop+=2;
	}
	if((logconf->policy & HasFault)) {
		buftop = writeKeyToBuffer(TEXTSIZE("FaultType"), buftop, bufend);
		writeToBuffer('"', buftop, bufend);
		if(TFLAG_is(int, logconf->policy, SystemFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SystemFault,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, ScriptFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("ScriptFault,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, DataFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("DataFault,"), buftop, bufend);
		}
		if(TFLAG_is(int, logconf->policy, ExternalFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("ExternalFault,"), buftop, bufend);
		}
//		if(TFLAG_is(int, logconf->policy, UnknownFault)) {
//			buftop = writeFixedTextToBuffer(TEXTSIZE("UnknownFault,"), buftop, bufend);
//		}
		buftop[-1] = '"';
		buftop[0] = ',';
		buftop[1] = ' ';
		buftop+=2;
	}
//	if(TFLAG_is(int, logconf->policy, PrivacyCaution)) {
//		buftop = writeTextToBuffer("PrivacyCaution\": \"true", buftop, bufend);
//		buftop[0] = ',';
//		buftop[1] = ' ';
//		buftop+=2;
//	}
	return buftop;
}

static char* writeErrnoToBuffer(logconf_t *logconf, char *buftop, char *bufend)
{
	if((logconf->policy & HasFault)) {
		buftop = writeKeyToBuffer(TEXTSIZE("Errno"), buftop, bufend);
		buftop = writeUnsingedIntToBuffer((uintptr_t)errno, buftop, bufend);
		buftop[0] = ','; buftop[1] = ' '; buftop+=2;
		buftop = writeKeyToBuffer(TEXTSIZE("Message"), buftop, bufend);
		buftop = writeTextToBuffer(strerror(errno), buftop, bufend);
		errno = 0;
	}
	return buftop;
}

static void writeDataLogToBuffer(logconf_t *logconf, va_list ap, char *buftop, char *bufend)
{
	int c = 0, logtype;
	buftop[0] = '{'; buftop++;
	buftop = writePolicyToBuffer(logconf, buftop, bufend);
	while((logtype = va_arg(ap, int)) != LOG_END) {
		if(c > 0 && buftop + 3 < bufend) {
			buftop[0] = ',';
			buftop[1] = ' ';
			buftop+=2;
		}
		switch(logtype) {
		case LOG_s: {
			const char *key = va_arg(ap, const char*);
			const char *text = va_arg(ap, const char*);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeTextToBuffer(text, buftop, bufend);
			break;
		}
		case LOG_u: {
			const char *key = va_arg(ap, const char*);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeUnsingedIntToBuffer(va_arg(ap, uintptr_t), buftop, bufend);
			break;
		}
		case LOG_ERRNO : {
			buftop = writeErrnoToBuffer(logconf, buftop, bufend);
			break;
		}
		}
		c++;
	}
	buftop[0] = '}'; buftop++;
	buftop[0] = '\0';
}

#endif /* LOGFORMATTER_H_ */
