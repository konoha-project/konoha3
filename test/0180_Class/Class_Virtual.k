// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C;

class C {
	int x;
	C(){}
}
void test() {
	C c = new C();
	assert(c.x == 0);
}

test();


