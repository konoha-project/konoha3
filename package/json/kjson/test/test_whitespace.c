#include "kjson.h"
#include "benchmark.h"
#include <stdlib.h>
#include <assert.h>

static void test_whitespace(char *data, size_t length)
{
    JSON json = parseJSON(data, data+length);
    assert(JSON_type(json) == JSON_Array);
    assert(JSON_length(json) == 1);
    JSON_free(json);
}

int main(int argc, char const* argv[])
{
    // whitespace test
    size_t i, length = 1024 * 1024;
    char *data = (char *)malloc(length  + 4);
    char *p = data;
    for (i = 0; i < length; i += 4) {
        *p++ = ' ';
        *p++ = '\n';
        *p++ = '\r';
        *p++ = '\t';
    }
    *p++ = '[';
    *p++ = '0';
    *p++ = ']';
    *p++ = '\0';

    reset_timer();
    for (i = 0; i < 128; i++) {
        test_whitespace(data, length+4);
    }
    show_timer("parse array with whitespace");
    free(data);
    return 0;
}
