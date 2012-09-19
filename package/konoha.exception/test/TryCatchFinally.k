// written by imasahiro
import("konoha.exception");
import("konoha.new");

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
