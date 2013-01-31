// written by unknown

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class A;

class B {
	int x;
	A a;
	B() {
		a = new A();
		x = a.add();
	}
}

class A {
	int x;
	int y;
	A() {
		x = 1;
		y = 2;
	}
	int add() {
		return x + y;
	}
}

void test() {
	B b = new B();
	assert(b.a.x == 1);
	assert(b.a.y == 2);
	assert(b.x == 3);
	assert(b.a.add() == 3);
}

test();
