// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x;
	int succ() {
		return x;
	}
}

class D extends C {
	@Override int succ() {
		return x + 1;
	}
}

void test() {
	C c = new C();
	D d = new D();
	assert(c.succ() == 0);
	assert(d.succ() == 1);
}

test();

