// written by shunsuke shida
K.import("konoha");

class A {
	int age;
	A() {
		age = 10;
	}
}

A a = new A();

assert(a.age == 10);
