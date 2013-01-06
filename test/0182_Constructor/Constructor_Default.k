// writte by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class Person {
	String name;
	int age;
}

void f() {
	Person p = new Person();
	assert(p.age == 0);
}

f();

