// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x;
	@Virtual int succ() {
		return x;
	}
}

void test() {
	C c = new C();
	assert(c.succ() == 0);
}

test();

