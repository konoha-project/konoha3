// written by shinpei

import("JavaScript.Array");
import("Syntax.JavaNewArray");
import("Syntax.JavaStyleClass");

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
	Person p1 = pa[0];
	Person p2 = pa[1];
	assert(p1.age == 17);
	assert(p2.age == 22);
	assert(p1.name == "naruto");
	assert(p2.name == "kakashi");
}

test();
