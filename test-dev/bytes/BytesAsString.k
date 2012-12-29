// written by imasahiro
// modified shinpei nakata

import("JavaScript.String");
import("konoha.bytes");
import("konoha.new");

void test() {
    String a  = "aaaaaaaaaaaaaaaa";
    String b  = "bbbbbbbbbbbbbbbb";
    String c  = a + b;
    Bytes  t  = c.toBytes();
    String ab = new String(t);
    assert("aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" == ab);
}

test();
