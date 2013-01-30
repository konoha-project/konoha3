// added by ide
Import("Syntax.JavaStyleClass");

class A {
    A(){}
    @Virtual int add(int x) {
        return x;
    }
}

class B extends A {
    B() {}
    @Virtual int add(int x) {
        return x + 1;
    }
}

class C extends A {
    C() {}
    @Virtual int add(int x) {
        return x + 2;
    }
}

void func(A a, int ans) {
    assert(a.add(10) == ans);
}

void main() {
    A a = new A();
    A b = new B();
    A c = new C();
    func(a, 10);
    func(b, 11);
    func(c, 12);
}

main();
