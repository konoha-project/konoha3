// written by kimio kuramitsu

import("Syntax.GlobalVariable");
//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

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

Person p = new Person("naruto", 17);
assert(p.isChild());

@Override boolean Person.isChild() {
	return this.age < 7;
}

assert(!p.isChild());

