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

#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

char *optarg = 0;
int optind   = 1;
int optopt   = 0;
int opterr   = 0;
int optreset = 0;

struct option {
	char *name;
	int has_arg;
	int *flag;
	int val;
};

/* The has_arg field should be one of: */
enum {
	no_argument,       /* no argument to the option is expect        */
	required_argument, /* an argument to the option is required      */
	optional_argument, /* an argument to the option may be presented */
};

static int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex)
{
	if(optind < argc) {
		char *arg = argv[optind];
		if(arg == 0)
			return -1;
		if(arg[0] == '-' && arg[1] == '-') {
			const struct option *opt = longopts;
			arg += 2;
			while(opt->name) {
				char *end = strchr(arg, '=');
				if(end == 0 && opt->has_arg == no_argument) {
					if(strcmp(arg, opt->name) == 0)
						*longindex = opt - longopts;
					optind++;
					return opt->val;
				}
				if(strncmp(arg, opt->name, end - arg) == 0) {
					*longindex = opt - longopts;
					optarg = end+1;
					optind++;
					return opt->val;
				}
				opt++;
			}
		}
		else if(arg[0] == '-') {
			arg += 1;
			const char *c = optstring;
			while(*c != 0) {
				if(*c == arg[0]) {
					if(*(c+1) == ':' && arg[1] == '=') {
						optarg = arg+2;
					}
					optind++;
					return arg[0];
				}
				c++;
			}
		}
	}
	return -1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
