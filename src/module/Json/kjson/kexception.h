/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org>
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

#include <setjmp.h>
#include <string.h>

#ifndef KEXCEPTION_H
#define KEXCEPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRY(HANDLER)  int jumped__ = 0; if((jumped__ = setjmp((HANDLER).handler)) == 0)
#define CATCH(X) if(jumped__ == (X))
#define FINALLY() L_finally:
#define THROW(HANDLER, EXCEPTION, ERROR_MSG) do {\
    (HANDLER)->error_message = (ERROR_MSG);\
    longjmp((HANDLER)->handler, (EXCEPTION));\
} while(0)

enum kjson_exception_type {
    KJSON_EXCEPTION     = 1,
    PARSER_EXCEPTION,
    STRINGIFY_EXCEPTION
};

typedef struct kexception_handler {
    jmp_buf handler;
    const char *error_message;
    int has_error;
} kexception_handler_t;

static inline void kexception_handler_reset(kexception_handler_t *eh)
{
    memset(eh->handler, 0, sizeof(jmp_buf));
    eh->has_error = 0;
}

static inline void kexception_handler_init(kexception_handler_t *eh)
{
    kexception_handler_reset(eh);
}

static inline int kexception_handler_deinit(kexception_handler_t *eh)
{
    kexception_handler_reset(eh);
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
