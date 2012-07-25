// written by kimio kuramitsu

K.import("konoha");

class Person {
	String name;
	int age;
}

void test() {
	Person[] a = new Person[10];
	assert(a.getSize() == 10);
}

test();

