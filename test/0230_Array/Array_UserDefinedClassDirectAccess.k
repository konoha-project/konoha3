// written by shinpei

import("JavaScript.Array");
import("Syntax.JavaStyleClass");
import("Syntax.JavaNewArray");

class Person {
	int age;
	String name;

	Person(int age, String name) {
		this.age = age;
		this.name = name;
	}
}

void test() {
	Person[] pa = new Person[2];
	pa[0] = new Person(17, "naruto");
	pa[1] = new Person(22, "kakashi");
	assert(pa[0].age == 17);
	assert(pa[1].age == 22);
	assert(pa[0].name == "naruto");
	assert(pa[1].name == "kakashi");
}

test();
