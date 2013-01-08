// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	C() {}
	String name = "naruto";
}

void test() {
	C c = new C();
	assert(c.name == "naruto");
}
test();

