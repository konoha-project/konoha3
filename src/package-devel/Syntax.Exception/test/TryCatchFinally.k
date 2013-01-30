// written by imasahiro
Import("Syntax.Exception");

void f() {
    System.p("try-start");
    try {
        System.p("try");
        System.throw(new Exception());
        System.p("unreachable");
    } catch (Exception e) {
        System.p("catched1");
    } catch (Exception e2) {
        System.p("catched2");
    } finally {
        System.p("finally");
    }
    System.p("try-end");
}

f();
