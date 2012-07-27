K.import("konoha");

class A{
	int num;
	A() {
		num = 10;
	}
}

void test_oprAssign(A a) {
	a.num += 1;
	p(a.num);
}

A a = new A();
p(a.num);
test_oprAssign(a);
