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

/* ************************************************************************ */

#include <getopt.h>
#include <logpool/logpool.h>
#include "util.h"
#include "dse.h"

#define HTTPD_ADDR "0.0.0.0"
#define HTTPD_PORT 8080

#ifdef __cplusplus
extern "C" {
#endif

int verbose_debug = 0;
int port = HTTPD_PORT;
char *logpoolip = NULL;

static struct option long_option[] = {
	/* These options set a flag. */
	{"verbose", no_argument, &verbose_debug, 1},
	{"logpool", required_argument, 0, 'l'},
	{"port", required_argument, 0, 'p'},
};

static void dse_parseopt(int argc, char *argv[])
{
	char *e;
	logpoolip = getenv("LOGPOOL_IP");
	while(1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "l:p:", long_option, &option_index);
		if(c == -1) break; /* Detect the end of the options. */
		switch(c) {
			case 'l':
				logpoolip = optarg;
				break;
			case 'p':
				port = (int)strtol(optarg, &e, 10);
				break;
			case '?':
				fprintf(stderr, "Unknown or required argument option -%c\n", optopt);
				fprintf(stderr, "Usage: COMMAND [ --verbose ] [ --port | -p ] port [ --logpool | -l ] logpoolip\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}
	if(!logpoolip) logpoolip = "0.0.0.0";
#ifdef DSE_DEBUG
	verbose_debug = 1;
#endif /* DSE_DEBUG */
}

int main(int argc, char *argv[])
{
	dse_parseopt(argc, argv);
	DEBUG_PRINT("DSE starts on port %d", port);
	DEBUG_PRINT("LogPool is running on %s", logpoolip);
	logpool_global_init(LOGPOOL_TRACE);
	DSE *dse = DSE_new();
	DSE_start(dse, HTTPD_ADDR, port);
	DSE_delete(dse);
	logpool_global_exit();
	return 0;
}

#ifdef __cplusplus
}
#endif
