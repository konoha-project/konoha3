// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C0 {int x0;}

class C1 extends C0 {int x1;}

void test() {
	C1 c = new C1();
	assert(c.x0 == 0);
	assert(c.x1 == 0);
}

test();
