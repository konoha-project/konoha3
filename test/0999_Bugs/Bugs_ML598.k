// written by imasahiro
// Reported at ML #598
// SEGV at git:c4b1feb97a58bef08bf1472fad346689cf9ac4a0
import("Syntax.Null");
import("JavaScript.Array");
import("JavaScript.String");

String[] extractNoun() {
    if(false) {
        return null;
    }
    String[] nouns = null;
    String[] lines = [];
    assert(lines.getlength()>=0);
    return lines;
}

void test() {
    String[] lines = extractNoun();
    assert(lines.toString() == "[]");
    assert(lines.getlength().toString() == "0");
}

test();
