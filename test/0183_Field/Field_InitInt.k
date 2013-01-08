// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int age = 10;
	C() {}
}

void test() {
	C c = new C();
	assert(c.age == 10);
}
test();

