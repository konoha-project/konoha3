#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#define USE_JSON_C
//#define USE_YAJL

#ifdef USE_MSGPACK
#include <msgpack.h>
#include <msgpack/pack.h>
#include <msgpack/unpack.h>
#endif
#ifdef USE_YAJL
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>
#endif
#include "kjson.h"
#include "benchmark.h"

static const unsigned int TASK_INT_NUM = 1<<24;
static const unsigned int TASK_STR_LEN = 1<<15;
//static const unsigned int TASK_INT_NUM = 1<<20;
//static const unsigned int TASK_STR_LEN = 1<<12;
static const char* TASK_STR_PTR;

void bench_json(void)
{
#ifdef USE_YAJL
    puts("== JSON ==");
    yajl_gen g = yajl_gen_alloc(NULL
#if YAJL_MAJOR < 2
            ,NULL
#endif
            );
    yajl_callbacks callbacks = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
    yajl_handle h = yajl_alloc(&callbacks, NULL,
#if YAJL_MAJOR < 2
            NULL,
#endif
            NULL);
    const unsigned char * buf;
    size_t len;
    reset_timer();
    {
        unsigned int i;
        yajl_gen_array_open(g);
        for(i=0; i < TASK_INT_NUM; ++i) {
            yajl_gen_integer(g, i);
        }
        yajl_gen_array_close(g);
    }
    show_timer("generate integer");

    yajl_gen_get_buf(g, &buf, &len);

    reset_timer();
    {
        yajl_status stat = yajl_parse(h, buf, len);
        if (stat != yajl_status_ok) {
            unsigned char * str = yajl_get_error(h, 1, buf, len);
            fprintf(stderr, "%s\n", (const char *) str);
        }
    }
    show_timer("parse integer");
    yajl_gen_free(g);
    g = yajl_gen_alloc(NULL
#if YAJL_MAJOR < 2
            ,NULL
#endif
            );
    yajl_free(h);
    h = yajl_alloc(&callbacks, NULL,
#if YAJL_MAJOR < 2
            NULL,
#endif
            NULL);
    reset_timer();
    {
        unsigned int i;
        yajl_gen_array_open(g);
        for(i=0; i < TASK_STR_LEN; ++i) {
            yajl_gen_string(g, (const unsigned char*)TASK_STR_PTR, i);
        }
        yajl_gen_array_close(g);
    }
    show_timer("generate string");

    yajl_gen_get_buf(g, &buf, &len);

    reset_timer();
    {
        yajl_status stat = yajl_parse(h, buf, len);
        if (stat != yajl_status_ok) {
            unsigned char * str = yajl_get_error(h, 1, buf, len);
            fprintf(stderr, "%s", (const char *) str);
        }
    }
    show_timer("parse string");
    yajl_gen_free(g);
    yajl_free(h);
#endif
}

void bench_msgpack(void)
{
#ifdef USE_MSGPACK
    puts("== MessagePack ==");
    msgpack_sbuffer *sbuf = msgpack_sbuffer_new();
    msgpack_packer* mpk = msgpack_packer_new(sbuf, msgpack_sbuffer_write);

    msgpack_unpacker mpac;
    msgpack_unpacker_init(&mpac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
    msgpack_unpacked msg;

    reset_timer();
    {
        unsigned int i;
        msgpack_pack_array(mpk, TASK_INT_NUM);
        for(i=0; i < TASK_INT_NUM; ++i) {
            msgpack_pack_unsigned_int(mpk, i);
        }
    }
    show_timer("pack integer");

    reset_timer();
    {
        size_t off = 0;
        bool ret = msgpack_unpack_next(&msg, sbuf->data, sbuf->size, &off);
        if (ret < 0) {
            fprintf(stderr, "Parse error.\n");
        } else if (ret == 0) {
            fprintf(stderr, "Not finished.\n");
        }
    }
    show_timer("unpack integer");

    msgpack_packer_free(mpk);
    msgpack_unpacker_reset(&mpac);
    mpk = msgpack_packer_new(sbuf, msgpack_sbuffer_write);
    reset_timer();
    {
        unsigned int i;
        msgpack_pack_array(mpk, TASK_STR_LEN);
        for(i=0; i < TASK_STR_LEN; ++i) {
            msgpack_pack_raw(mpk, i);
            msgpack_pack_raw_body(mpk, TASK_STR_PTR, i);
        }
    }
    show_timer("pack string");

    reset_timer();
    {
        size_t off = 0;
        bool ret = msgpack_unpack_next(&msg, sbuf->data, sbuf->size, &off);
        if (ret < 0) {
            fprintf(stderr, "Parse error.\n");
        } else if (ret == 0) {
            fprintf(stderr, "Not finished.\n");
        }
    }
    show_timer("unpack string");
#endif
}

void bench_kjson(void)
{
    puts("== KJSON ==");

    JSON o;
    char *buf;
    size_t len;
    reset_timer();
    {
        unsigned int i;
        o = JSONArray_new();
        for(i=0; i < TASK_INT_NUM; ++i) {
            JSON v = JSONInt_new(i);
            JSONArray_append(o, v);
        }
    }
    show_timer("generate integer");

    buf = JSON_toStringWithLength(o, &len);
    JSON_free(o);

    reset_timer();
    {
        o = parseJSON(buf, buf + len);
        if (o.bits == 0) {
            fprintf(stderr, "Error\n");
        }
    }
    show_timer("parse integer");

    JSON_free(o);
    free(buf);

    reset_timer();
    {
        unsigned int i;
        o = JSONArray_new();
        for(i=0; i < TASK_STR_LEN; ++i) {
            JSON v = JSONString_new((char*)TASK_STR_PTR, i);
            JSONArray_append(o, v);
        }
    }
    show_timer("generate string");
    buf = JSON_toStringWithLength(o, &len);
    JSON_free(o);

    reset_timer();
    {
        o = parseJSON(buf, buf + len);
        if (o.bits == 0) {
            fprintf(stderr, "Errro\n");
        }
    }
    show_timer("parse string");
    JSON_free(o);
    free(buf);
}

int main(int argc, char* argv[])
{
    char* str = malloc(TASK_STR_LEN);
    memset(str, 'a', TASK_STR_LEN);
    TASK_STR_PTR = str;
    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    } else {
        bench_msgpack();
        bench_json();
    }
    bench_kjson();
    free(str);
    return 0;
}
