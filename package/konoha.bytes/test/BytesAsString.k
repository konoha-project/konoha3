// written by imasahiro
// modified shinpei nakata

import("konoha.string");
import("konoha.bytes");

void test() {
    String a  = "aaaaaaaaaaaaaaaa";
    String b  = "bbbbbbbbbbbbbbbb";
    String c  = a + b;
    Bytes  t  = c.toBytes();
	String ab = new String(t);
    assert("aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" == ab);
}

test();
