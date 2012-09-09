// written by imasahiro
K.import("konoha.exception");
K.import("konoha.new");

System.p("try-start");
try {
    System.p("try");
    System.throw(new Exception());
    System.p("unreachable");
} catch (Exception e) {
    System.p("catched");
} finally {
    System.p("finally");
}
System.p("try-end");
