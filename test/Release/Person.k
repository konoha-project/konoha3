// writte by kimio kuramitsu

import("konoha.class");
// import("konoha.assign"); Don't use any more

class Person {
	String name;
	int age;
	Person(String name, int age) {
		this.name = name;
		this.age = age;
	}
}

void f() {
	Person p = new Person("naruto", 17);
	assert(p.age == 17);
}

f();

