// writte by kimio kuramitsu

import("konoha.class");

class Person {
	String name;
	int age;
}

void f() {
	Person p = new Person();
	assert(p.age == 0);
}

f();

