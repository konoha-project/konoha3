#include <stdio.h>

int main() {
    int i;
    for (i=0;i<256;i++) {
        if (i > 0 && i % 16 == 0) {
            fprintf(stderr, "\n");
        }
        switch (i) {
            case '{':
                fprintf(stderr, "1   ,");
                break;
            case '"':
                fprintf(stderr, "_M 2,");
                break;
            case '[':
                fprintf(stderr, "3   ,");
                break;
            case '0':case '1':case '2':case '3':case '4':case '5':
            case '6':case '7':case '8':case '9':case '-':
                fprintf(stderr, "4   ,");
                break;
            case 't':case 'f':
                fprintf(stderr, "5   ,");
                break;
            case 'n':
                fprintf(stderr, "6   ,");
                break;
            case '\\':
                fprintf(stderr, "_M 0,");
                break;
            case ' ':case '\r':case '\n':case '\t':
                fprintf(stderr, "_N 0,");
                break;

            default:
                fprintf(stderr, "0   ,");
                break;
        }
    }
    fprintf(stderr, "\n");
}
