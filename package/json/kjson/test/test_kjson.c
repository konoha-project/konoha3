#include "kjson.h"
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static char *loadFile(const char *file, size_t *file_len)
{
    size_t len = 1024, size, offset = 0;
    char *str = malloc(1024);
    char *end = str + 1024;
    FILE *fp = fopen(file, "r");
    char buf[1024];
    assert(fp);
    while (1) {
        size = fread(buf, 1, 1024, fp);
        if (size == 0)
            break;
        if (str + offset + size > end) {
            len *= 2;
            str = realloc(str, len);
            end = str + len;
        }
        memcpy(str+offset, buf, size);
        offset += size;
    }
    *file_len = offset;
    fclose(fp);
    return str;
}

static void test_file(const char *file)
{
    fprintf(stderr, "--- {{ test %s --- \n", file);
    size_t len;
    char *str = loadFile(file, &len);
    JSON json = parseJSON(str, str+len);
    size_t json_len;
    char *json_s = JSON_toStringWithLength(json, &json_len);
    fprintf(stderr, "'%s'\n", json_s);
    fprintf(stderr, "\n--- }} test %s --- \n", file);
    free(json_s);
    JSON_free(json);
    free(str);
}

static const char *names[] = {
    "foo", "bar", "mono"
};
static const int lines[] = {
    10, 20, -40
};
static const double versions[] = {
    0.2, 0.3, 0.4
};

static char data[] =
"{\"app\": [\n"
"    {\n"
"        \"name\": \"foo\",\n"
"        \"line\": 10,\n"
"        \"version\": 0.2,\n"
"        \"flag\": true\n"
"    },\n"
"    {\n"
"        \"name\": \"bar\",\n"
"        \"line\": 20,\n"
"        \"version\": 0.3,\n"
"        \"flag\": true\n"
"    },\n"
"    {\n"
"        \"name\": \"mono\",\n"
"        \"line\": -40,\n"
"        \"version\": 0.4,\n"
"        \"flag\": null\n"
"    }]\n"
"}";

static void test_string(void)
{
    JSON json = parseJSON(data, data+sizeof(data));
    JSON child = JSON_get(json, "app");
    assert(JSON_isValid(child));
    assert(JSON_type(child) == JSON_Array);
    assert(JSON_length(child) == 3);
    JSONArray *a;
    JSON *I, *E;
    int i = 0;

    JSON_ARRAY_EACH(child, a, I, E) {
        assert(JSON_isValid(JSON_get(*I, "name")));
        assert(JSON_isValid(JSON_get(*I, "line")));
        assert(JSON_isValid(JSON_get(*I, "version")));
        assert(JSON_isValid(JSON_get(*I, "flag")));
        size_t len;
        const char *name = JSON_getString(*I, "name", &len);
        int   line = JSON_getInt(*I, "line");
        double ver  = JSON_getDouble(*I, "version");
        assert(len == strlen(names[i]) && strncmp(name, names[i], len) == 0);
        assert(line == lines[i]);
        assert(ver  == versions[i]);
        fprintf(stderr, "%s %d %f\n", name, line, ver);
        i++;
    }
    JSON_free(json);
}

static char data2[] = 
"{\n"
"    \"name\": \"foo\",\n"
"    \"line\": 10,\n"
"    \"version\": 0.2,\n"
"    \"flag\": true\n"
"}\n";

static void test_object_iterator(void)
{
    JSON o = parseJSON(data2, data2+sizeof(data2));
    assert(JSON_type(o) == JSON_Object);
    //assert(JSON_length((JSON)child) == 3);

    JSON Key;
    JSON Val;
    JSONObject_iterator Itr;

    JSON_OBJECT_EACH(o, Itr, Key, Val) {
        char *k, *v;
        const char *str = JSONString_get(Key);
        assert(JSON_type(Key) == JSON_String);
        assert(JSON_type(Val) == JSON_type(JSON_get(o, str)));
        k = JSON_toString(Key);
        v = JSON_toString(Val);
        fprintf(stderr, "<'%s':'%s'>", k, v);
        free(k); free(v);
    }
    JSON_free((JSON)o);
}

int main(int argc, char const* argv[])
{
    const char **files;
    const char *files_default[] = {
        "./test/test01.json",
        "./test/test02.json",
        "./test/test03.json",
        "./test/test04.json",
        "./test/test05.json",
        "./test/test06.json",
        "./test/test07.json",
        "./test/test08.json",
        "./test/test09.json",
        "./test/test10.json",
        "./test/test11.json",
        "./test/test12.json",
        "./test/test13.json",
        "./test/array1000.json",
        "./test/simple.json",
        "./test/benchmark1.json",
        "./test/benchmark2.json",
        "./test/benchmark3.json",
        "./test/benchmark4.json",
        "./test/benchmark5.json",
        "./test/delicious_popular.json",
        "./test/twitter_public.json",
        "./test/lastfm.json",
        "./test/yelp.json",
    };
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif
    size_t size = ARRAY_SIZE(files_default);
    if (argc > 1) {
        files = argv+1;
        size  = argc-1;
    } else {
        files = files_default;
    }
    int i;
    for (i = 0; i < size; i++) {
        test_file(files[i]);
    }
    test_string();
    test_object_iterator();
    return 0;
}
