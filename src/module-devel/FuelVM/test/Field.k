// written by shunsuke shida
// modified by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int age;
	C () {
	}
}

void test() {
	C c = new C();
	assert(c.age == 0);
}

test();

