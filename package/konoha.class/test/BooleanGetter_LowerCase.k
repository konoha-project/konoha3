// writte by kimio kuramitsu

import("konoha.class");
import("konoha.assign");

class Person {
	String name;
	int age;
	boolean child;
	Person(String name, int age) {
		this.name = name;
		this.age = age;
	}
	boolean isChild() {
		return child;
	}
}

void f() {
	Person p = new Person("naruto", 17);
	assert(!(p.getchild()));
	assert(!(p.isChild()));
}

f();

