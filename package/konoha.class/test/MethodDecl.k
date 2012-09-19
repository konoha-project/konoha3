// written by kimio kuramitsu

import("konoha.class");
import("konoha.assign");

class Person {
	String name;
	int age;
	Person(String name, int age) {
		this.name = name;
		this.age = age;
	}
	boolean isChild() {
		return this.age < 20;
	}
}

void test() {
	Person p = new Person("naruto", 17);
	assert(p.isChild());
}

test();

