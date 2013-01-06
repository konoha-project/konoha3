// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x;
	C() {
		x = 1;
	}
}

void test() {
	C c = new C();
	assert(c.x==1);
}

test();


