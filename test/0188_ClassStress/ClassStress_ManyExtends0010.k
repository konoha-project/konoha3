// written by kimio kuramitsu

import("Syntax.JavaStyleClass");

class C0 {
	int x0;
	C0() {}
}

class C1 extends C0 {
	int x1;
}

class C2 extends C1 {
	int x2;
}

class C3 extends C2 {
	int x3;
}

class C4 extends C3 {
	int x4;
}

class C5 extends C4 {
	int x5;
}

class C6 extends C5 {
	int x6;
}

class C7 extends C6 {
	int x7;
}

class C8 extends C7 {
	int x8;
}

class C9 extends C8 {
	int x9;
}


void test() {
	C9 c = new C9();
	assert(c.x0 == 0);
	assert(c.x1 == 0);
	assert(c.x2 == 0);
	assert(c.x3 == 0);
	assert(c.x4 == 0);
	assert(c.x5 == 0);
	assert(c.x6 == 0);
	assert(c.x7 == 0);
	assert(c.x8 == 0);
	assert(c.x9 == 0);

}

test();
