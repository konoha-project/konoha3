// written by imasahiro
// Reported at ML #592
// SEGV at git:01cdf4c9d7504c9d98fc1412556481b665eb4922
//FIXME(imasahiro) Move this test case the right directory
import("JavaScript.Array");
import("JavaScript.String");

int test(String src) {
    if(false) {
        return 1;
    }
    String nouns = "abcd";
    String[] lines = src.split("\n");
    assert(nouns.getlength() == 4);
    return 0;
}

"aaa\nbbb".split("\n"); /* <- Compiling String.split(String); */
test("aaa\nbbb");
