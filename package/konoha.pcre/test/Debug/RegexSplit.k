K.import("konoha.pcre");
K.import("konoha.array");

void f() {
    Array[String] r1 = "aa/bb/cc/dd/ee".split(/\//);
    System.p(r1[0]);
    System.p(r1[1]);
    System.p(r1[2]);
    System.p(r1[3]);
    System.p(r1[4]);
    Array[String] r2 = "hoge1234hoge1234".split(/[gh]/);
    System.p(r2[0]);
    System.p(r2[1]);
    System.p(r2[2]);
    System.p(r2[3]);
    System.p(r2[4]);
}

f();
