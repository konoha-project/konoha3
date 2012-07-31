// writte by kimio kuramitsu

K.import("konoha.class");
K.import("konoha.assign");

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
	p.setAge(18);
	assert(p.age == 18);
}

f();

