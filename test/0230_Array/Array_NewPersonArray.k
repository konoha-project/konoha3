// written by kimio kuramitsu
import("Syntax.JavaNewArray");
import("Syntax.JavaStyleClass")
import("JavaScript.Array");

class Person {
	String name;
	int age;
}

void test() {
	Person[] a = new Person[10];
	assert(a.getSize() == 10);
}

test();

