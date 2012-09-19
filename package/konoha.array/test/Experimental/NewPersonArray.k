// written by kimio kuramitsu
import("konoha.new")
import("konoha.class")
import("konoha.array");

class Person {
	String name;
	int age;
}

void test() {
	Person[] a = new Person[10];
	assert(a.getSize() == 10);
}

test();

