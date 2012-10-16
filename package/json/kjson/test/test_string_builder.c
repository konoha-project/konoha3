#include "kstring_builder.h"
#include <stdio.h>
int main(int argc, char const* argv[])
{
    string_builder builder;
    int i;
    string_builder_init(&builder);
    for (i = 0; i < 10; ++i) {
        string_builder_add(&builder, i+'a');
    }
    size_t len;
    char *s = string_builder_tostring(&builder, &len, 1);
    fprintf(stderr, "'%s'\n", s);
    free(s);
    return 0;
}
