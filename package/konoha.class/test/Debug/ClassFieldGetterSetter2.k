// written by imasahiro

K.import("konoha");
K.import("konoha.float");
K.import("konoha.class");

class A {
	int ival;
}

int f() {
    A a = new A();
    return a.ival;
}

class B {
	String stringValue;
}

String g() {
    B b = new B();
    return b.stringValue;
}

class C {
    boolean bval;
}

boolean h() {
    C c = new C();
    return c.bval;
}

class D {
	float fval;
}

int i() {
    D d = new D();
    return d.fval;
}

void x() {
    assert(0     == f());
    assert(""    == g());
    assert(false == h());
    assert(0     == i());
}

x();
