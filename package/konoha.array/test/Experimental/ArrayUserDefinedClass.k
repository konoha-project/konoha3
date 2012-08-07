// written by shinpei
K.import("konoha.new");
K.import("konoha.class");
K.import("konoha.array");
K.import("konoha.assign");

class Human {
	int age;
	String name;

	Human(int a) {
		age = a;
	}
}

void f() {
	Human[] ha = new Human[2];
	ha[0] = new Human(17);
	ha[1] = new Human(22);
	assert(ha[0].age == 17);
	assert(ha[1].age == 22);

}

f();
