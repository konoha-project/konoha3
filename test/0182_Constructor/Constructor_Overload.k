// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x;
	int y;
	C(int x1) {
		x = x1;
	}
	C(int x2, int y2) {
		x = x2;
		y = y2;
	}
}

void test() {
	C c1 = new C(1);
	C c2 = new C(2, 3);
	assert(c1.x==1);
	assert(c2.x==2);
	assert(c2.y==3);
}

test();


