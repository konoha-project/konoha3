// written by imasahiro
import("konoha.string");
import("konoha.bytes");

void f() {
    String a = "aaaaaaaaaaaaaaaa";
    String b = "bbbbbbbbbbbbbbbb";
    String c = a + b;
    Bytes  t = c.toBytes();
    assert("aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" == t.toString());
}

f();
