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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "kexception.h"
#include "kstack.h"

#ifndef KJSON_STREAM_H
#define KJSON_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct input_stream {
    const uint8_t *pos;
    const uint8_t *end;
    kstack_t stack;
    kexception_handler_t exception;
} input_stream;

static inline const uint8_t *_input_stream_save(input_stream *ins)
{
    return ins->pos;
}

static inline void _input_stream_resume(input_stream *ins, const uint8_t *pos)
{
    ins->pos = pos;
}

static inline uint8_t string_input_stream_next(input_stream *ins)
{
    return *(ins->pos)++;
}

static inline bool string_input_stream_eos(input_stream *ins)
{
    return ins->pos != ins->end;
}

#define for_each_istream(INS, CUR)\
    for(CUR = string_input_stream_next(INS);\
            string_input_stream_eos(INS); CUR = string_input_stream_next(INS))

static input_stream *new_string_input_stream(input_stream *ins, const char *buf, size_t len)
{
    const uint8_t *text = (const uint8_t *) buf;
    ins->pos = text;
    ins->end = text + len + 1;
    kexception_handler_init(&ins->exception);
    kstack_init(&ins->stack);
    return ins;
}

static void string_input_stream_deinit(input_stream *ins)
{
    ins->pos = ins->end = NULL;
    kstack_deinit(&ins->stack, ins->exception.has_error == 0);
}

static void input_stream_delete(input_stream *ins)
{
    string_input_stream_deinit(ins);
}

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
