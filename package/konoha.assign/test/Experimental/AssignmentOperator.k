// written by kimio kuramitsu

K.import("konoha.assign");

void test() {
	int a = 1;
	int b = 2;
	int c = a = b;
	System.p("a=" + a);
	System.p("b=" + b);
	System.p("c=" + c);
	assert(a == 2);
	assert(b == 2);
	assert(c == 2);
}

test();

	
