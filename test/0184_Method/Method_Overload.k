// written by kimio kuramitsu

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
	boolean isChild(int age) {   // @Test Overloading
		return this.age < age;
	}
}

void test() {
	Person p = new Person("naruto", 17);
	assert(p.isChild());
	assert(p.isChild(19));
	assert(!p.isChild(7));      // This must be false
}

test();

