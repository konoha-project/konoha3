// written by imasahiro
// modified shinpei nakata

import("Java.String");
import("Type.Bytes");
//import("konoha.new"); Don't use this

void test() {
    String a  = "aaaaaaaaaaaaaaaa";
    String b  = "bbbbbbbbbbbbbbbb";
    String c  = a + b;
    Bytes  t  = c.toBytes();
    String ab = new String(t);
    assert("aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" == ab);
}

test();
