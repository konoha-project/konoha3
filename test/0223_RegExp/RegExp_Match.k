import("JavaScript.Array");
import("JavaScript.Regexp");

void f() {
    Array[String] r1 = "hoge1234".match(/123/);
    assert(r1[0] == "123");
    Array[String] r2 = "hoge1234".match(/\d/);
    assert(r2[0] == "1");
    Array[String] r3 = "hoge1234".match(/x/);
    Array[String] r4 = "hoge/1234".match(/\//);
    assert(r4[0] == "/");
    Array[String] r5 = "hoge1234".match(/1.*/);
    assert(r5[0] == "1234");
    Array[String] r6 = "hoge1234".match(/(h[ox]g[ae])(\d*)/);
    assert(r6[0] == "hoge1234");
    assert(r6[1] == "hoge");
    assert(r6[2] == "1234");
}

f();
