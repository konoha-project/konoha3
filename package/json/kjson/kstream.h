/****************************************************************************
 * Copyright (c) 2012, Masahiro Ide <ide@konohascript.org>
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef KJSON_STREAM_H
#define KJSON_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

struct input_stream;

union io_data {
    unsigned char *str;
    FILE *fp;
    uintptr_t u;
    void *ptr;
};

typedef struct input_stream {
    union io_data d0;
    union io_data d1;
    union io_data d2;
    long flags;
} input_stream;

static inline union io_data _input_stream_save(input_stream *ins)
{
    return ins->d0;
}

static inline void _input_stream_resume(input_stream *ins, union io_data data)
{
    ins->d0 = data;
}

static inline char string_input_stream_next(input_stream *ins)
{
    return *(ins->d0.str)++;
}

static inline bool string_input_stream_eos(input_stream *ins)
{
    return ins->d0.str != ins->d1.str;
}

#define for_each_istream(INS, CUR)\
    for (CUR = string_input_stream_next(INS);\
            string_input_stream_eos(INS); CUR = string_input_stream_next(INS))

static void string_input_stream_init(input_stream *ins, void **args)
{
    unsigned char *text;
    size_t len;
    text = (unsigned char *) args[0];
    len  = (size_t) args[1];
    ins->d0.str = text;
    ins->d1.str = text + len + 1;
    ins->d2.u   = 0;
}

static inline input_stream *new_input_stream(void **args, long flags)
{
    input_stream *ins = (input_stream *) KJSON_MALLOC(sizeof(input_stream));
    string_input_stream_init(ins, args);
    return ins;
}

static void string_input_stream_deinit(input_stream *ins)
{
    ins->d0.str = ins->d1.str = NULL;
}

static input_stream *new_string_input_stream(const char *buf, size_t len, long flags)
{
    void *args[] = {
        (void*)buf, (void*)len
    };
    input_stream *ins = new_input_stream(args, flags);
    return ins;
}

static void input_stream_delete(input_stream *ins)
{
    string_input_stream_deinit(ins);
    KJSON_FREE(ins);
}

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
