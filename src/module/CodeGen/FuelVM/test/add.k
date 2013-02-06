//@@ const + const
int add0() {
    return 10 + 100;
}

System.p(add0());

//@@ arg + const
int add1(int n) {
    return n + 100;
}

System.p(add1(10));

//@@ const + arg
int add2(int n) {
    return 10 + n;
}

System.p(add2(100));

//@@ const + const + arg
int add3(int n) {
    return 1 + 9 + n;
}

System.p(add3(100));

//@@ const + const + .... + const + arg
int add4(int n) {
    return 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + n;
}

System.p(add4(100));

//@@ local + const
int add5() {
    int n = 10;
    return n + 100;
}

System.p(add5());

//@@ local + local + arg
int add6(int n) {
    int a0 = 5;
    int a1 = 5;
    return a0 + a1 + n;
}

System.p(add6(100));

//@@ local + arg + const
int add7(int n) {
    int m = 1;
    return n + m + 99;
}

System.p(add7(10));


////@@ local + local + .... + local + arg
//int add8(int n) {
//    int a0 = 1;
//    int a1 = 1;
//    int a2 = 1;
//    int a3 = 1;
//    int a4 = 1;
//    int a5 = 1;
//    int a6 = 1;
//    int a7 = 1;
//    int a8 = 1;
//    int a9 = 1;
//    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + n;
//}
//
//System.p(add8(100));

//@@ const + local with branch
int add9(int n) {
    int m = 2;
    if(true) {
        m = 100;
    } else {
        m = 10;
    }
    return n + m;
}

System.p(add9(10));

//@@ const + local with nested branch
int add10(int n) {
    int m = 1;
    if(false) {
        m = 1000000;
    } else {
        m = 10000000;
        if(false) {
            m = 10000;
        } else {
            m = 100;
        }
    }
    return n + m;
}

System.p(add10(10));

