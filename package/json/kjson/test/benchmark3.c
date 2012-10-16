#include "kjson.h"
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "benchmark.h"

#define SIZE 1024*16
static char *loadLine(FILE *fp, char *buf)
{
    bzero(buf, SIZE);
    return fgets(buf, SIZE, fp);
}
#define JSON_FILE "./test/twitter.json"
static int filesize = 0;
static void test0()
{
    size_t len;
    char buf[SIZE], *str;
    FILE *fp = fopen(JSON_FILE, "r");
    reset_timer();
    while ((str = loadLine(fp, buf)) != NULL) {
        len = strlen(str);
        filesize += len;
    }
    show_timer("nop");
    fclose(fp);
}

static void test1()
{
    size_t len;
    char buf[SIZE], *str;
    FILE *fp = fopen(JSON_FILE, "r");
    reset_timer();
    while ((str = loadLine(fp, buf)) != NULL) {
        len = strlen(str);
        JSON json = parseJSON(str, str+len);
        assert(JSON_type(json) == JSON_Object);
        JSON_free(json);
    }
    _show_timer("parse", filesize);
    fclose(fp);
}

static void test2()
{
    size_t len;
    char buf[SIZE], *str;
    FILE *fp = fopen(JSON_FILE, "r");
    reset_timer();
    while ((str = loadLine(fp, buf)) != NULL) {
        len = strlen(str);
        JSON json = parseJSON(str, str+len);
        char *p = JSON_toStringWithLength(json, &len);
        assert(JSON_type(json) == JSON_Object);
        assert(p);
        JSON_free(json);
        free(p);
    }
    _show_timer("pack/unpack", filesize);
    fclose(fp);
}

int main(int argc, char const* argv[])
{
    int i, size = 1;
    fprintf(stderr, "benchmark3\n");
    for (i = 0; i < size; i++) {
        test0();
        test1();
        test2();
    }
    return 0;
}
