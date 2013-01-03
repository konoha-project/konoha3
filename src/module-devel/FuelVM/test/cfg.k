int foo(int a, int b) {
    int k = 1;          // k is constant: 1
    int m, n;
    if(a == 0) {
        k = k + 1;       // k is constant: 2
        m = a;
        n = b;
    } else {
        k = 2;          // k is constant: 2
        m = 0;
        n = a + b;
    }
    return k + m + n;   // k is constant: 2
}

System.p(foo(1, 2));
