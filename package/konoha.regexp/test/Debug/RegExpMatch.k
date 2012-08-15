K.import("konoha.new");
K.import("konoha.regexp");
K.import("konoha.array");

void f() {
    Array[String] r1 = "hoge1234".match(/123/);
    System.p(r1[0]);
    Array[String] r2 = "hoge1234".match(/\d/);
    System.p(r2[0]);
    Array[String] r3 = "hoge1234".match(/x/);
    Array[String] r4 = "hoge/1234".match(/\//);
    System.p(r4[0]);
    Array[String] r5 = "hoge1234".match(/1.*/);
    System.p(r5[0]);
    Array[String] r6 = "hoge1234".match(/(h[ox]g[ae])(\d*)/);
    System.p(r6[0]);
    System.p(r6[1]);
    System.p(r6[2]);
}

f();
