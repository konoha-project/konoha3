// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C;

class C {
	int x0;
	int x1;
	int x2;
	int x3;
	int x4;
	int x5;
	int x6;
	int x7;
	C(){}
}
void test() {
	C c = new C();
	assert(c.x0 == 0);
	assert(c.x7 == 0);
}

test();


