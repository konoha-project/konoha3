// written by imasahiro
import("ijit");

int fibo(int n) {
    if(n < 3) {
        return 1;
    }
    return fibo(n-1)+fibo(n-2);
}

int fibo2(int n) {
    if (n < 3) {
        return 1;
    } else {
        return fibo2(n-1) + fibo2(n-2);
    }
}

void f() {
    System.p(fibo(1));
    System.p(fibo(2));
    System.p(fibo(3));
    System.p(fibo(4));
    System.p(fibo(5));
    System.p(fibo(6));
    System.p(fibo(7));
    System.p(fibo(8));
    System.p(fibo(9));
    System.p(fibo(10));
    System.assert(fibo(10) == 55);
    System.p(fibo2(10));
    System.assert(fibo2(10) == 55);
}

f();
