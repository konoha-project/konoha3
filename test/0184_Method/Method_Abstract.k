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
	boolean isChild();    // @Test Abstract Method
}

void test() {
	Person p = new Person("naruto", 17);
	assert(p.isChild() == false);
}

test();
