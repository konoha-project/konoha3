// written by imasahiro
import("konoha.string");
import("konoha.bytes");

void f() {
    String a = "aaaaaaaaaaaaaaaa";
    String b = "bbbbbbbbbbbbbbbb";
    String c = a + b;
    Bytes  t = c.asBytes();
    assert("aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" == t.asString());
}

f();
