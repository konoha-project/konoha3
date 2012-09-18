// written by kimio kuramitsu
K.import("konoha.new")
K.import("konoha.class")
K.import("konoha.array");

class Person {
	String name;
	int age;
}

void test() {
	Person[] a = new Person[10];
	assert(a.getSize() == 10);
}

test();

