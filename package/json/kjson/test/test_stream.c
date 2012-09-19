#include "kjson.h"
#include "kstream.h"
#include <unistd.h>
#include <assert.h>

static void test_string(void)
{
    char buf[1024] = {0};
    int i;
    int ans = 0;
    for (i = 0; i < 1024; i++) {
        buf[i] = i;
        ans += (char)i;
    }
    input_stream *ins = new_string_input_stream((const char*)buf, 1024, 0);
    int sum = 0;
    char cur;
    for_each_istream(ins, cur) {
        sum += cur;
    }
    assert(sum == ans);
    input_stream_delete(ins);
}

//static void test_file(int argc, const char **argv)
//{
//    int ans = 0, sum = 0;
//    const char **files;
//    size_t size = argc;
//    if (argc > 0) {
//        files = (const char **) argv;
//    } else {
//        const char *files_default[] = {
//            "./test/test01.json",
//            "./test/test02.json",
//            "./test/test03.json",
//            "./test/test04.json",
//            "./test/test05.json",
//            "./test/test06.json",
//            "./test/test07.json",
//            "./test/test08.json",
//            "./test/test09.json",
//            "./test/benchmark1.json",
//            "./test/benchmark2.json",
//            "./test/benchmark3.json",
//            "./test/benchmark4.json",
//            "./test/benchmark5.json",
//            "./test/benchmark6.json",
//            "./test/benchmark7.json",
//        };
//        files = files_default;
//#ifndef ARRAY_SIZE
//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
//#endif
//        size = ARRAY_SIZE(files);
//    }
//    int i;
//    for (i = 0; i < size; i++) {
//        FILE *fp;
//        if ((fp = fopen(files[i], "r")) != NULL) {
//            char buf[1024];
//            size_t len;
//            while ((len = fread(buf, 1, sizeof(buf), fp)) > 0) {
//                int j;
//                for (j = 0; j < len; j++) {
//                    ans += (char)buf[j];
//                }
//            }
//            fclose(fp);
//        }
//
//        input_stream *ins = new_file_input_stream((char*)files[i], 1024);
//        input_stream_iterator itr = {};
//        char cur;
//        for_each_istream(ins, itr, cur) {
//            sum += cur;
//        }
//        fprintf(stderr, "file:%s, ans=%d, sum=%d\n", files[i], ans, sum);
//        assert(sum == ans);
//        input_stream_delete(ins);
//    }
//}

int main(int argc, char const* argv[])
{
    test_string();
    //test_file(argc-1, argv+1);
    return 0;
}
