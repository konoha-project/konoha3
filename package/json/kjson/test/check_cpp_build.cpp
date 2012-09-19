#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../kjson.c"

int main(int argc, char const* argv[])
{
#define JSON_FILE "test/simple.json"
    FILE *fp = fopen(JSON_FILE, "r");
    if (!fp)
        fp = fopen("../" JSON_FILE, "r");
    assert(fp != 0);

    fseek(fp, 0, SEEK_END);
    size_t len = (size_t) ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *json = (char *) calloc(1, len + 1);
    size_t readed = fread(json, 1, len, fp);
    assert(len == readed);
    fclose(fp);

    JSON o = parseJSON(json, json + len);
    assert(o.bits != 0);
    if (!JSON_isValid(o)) {
        fprintf(stderr, "Error\n");
        exit(EXIT_FAILURE);
    }
    JSON a = JSON_get(o, "a");
    assert(a.bits != 0);
    JSON_free(o);
    free(json);
    return 0;
}
