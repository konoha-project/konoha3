import("ijit");
int g(int x) {
    if (x >= 10) {
        if (x > 100) {
            return 10;
        } else {
            return 20;
        }
    } else {
        return 30;
    }
}
int f() {
    System.p(g(1000));
    System.p(g(10));
    System.p(g(1));
    System.assert(g(1000) == 10);
    System.assert(g(10) == 20);
    System.assert(g(1) == 30);
    return 0;
}

f();
